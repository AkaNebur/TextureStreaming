#include <iostream>
#include <queue>
#include <memory>
#include <stdexcept>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include <sstream>
#include <cassert>
#include <mutex>
#include <condition_variable>

class Texture2D {
public:
    int width;
    int height;

    Texture2D(int width, int height) : width(width), height(height) {}

    std::vector<unsigned char> ReadPixels(int x, int y, int w, int h) {
        if (x < 0 || y < 0 || x + w > width || y + h > height) {
            throw std::out_of_range("Read area is out of texture bounds!");
        }
        return std::vector<unsigned char>(w * h * 3, 0);
    }

    std::vector<unsigned char> EncodeToJPG(int quality) {
        return std::vector<unsigned char>();
    }
};

class TexturePool {
private:
    std::queue<std::shared_ptr<Texture2D>> availableTextures;
    std::mutex queueMutex;

public:
    std::shared_ptr<Texture2D> GetTexture(int width, int height) {
        std::lock_guard<std::mutex> lock(queueMutex);
        std::queue<std::shared_ptr<Texture2D>> tempQueue;

        while (!availableTextures.empty()) {
            auto texture = availableTextures.front();
            availableTextures.pop();

            if (texture->width == width && texture->height == height) {
                while (!tempQueue.empty()) {
                    availableTextures.push(tempQueue.front());
                    tempQueue.pop();
                }
                return texture;
            }
            else {
                tempQueue.push(texture);
            }
        }


        return std::make_shared<Texture2D>(width, height);
    }

    void ReturnTexture(std::shared_ptr<Texture2D> texture) {
        std::lock_guard<std::mutex> lock(queueMutex);
        availableTextures.push(std::move(texture));
    }
};

class PhotonNetwork {
public:
    static bool IsConnected;
    static bool IsMasterClient;

    static void RaiseEvent(unsigned char eventCode, const std::vector<unsigned char>& eventData) {}
};

bool PhotonNetwork::IsConnected = false;
bool PhotonNetwork::IsMasterClient = false;

class MonoBehaviourPunCallbacks {};

class TextureStreamer : public MonoBehaviourPunCallbacks {
private:
    TexturePool texturePool;

    enum FrameRatePreset { _20FPS, _24FPS, _30FPS, _60FPS };
    FrameRatePreset frameRate = _30FPS;

    int desiredResolutionHeight = 620;
    int compression = 30;
    int textureWidth;
    int textureHeight;

    float waitTime;
    std::atomic<bool> isStreaming{ true };
    std::condition_variable cv;
    std::mutex cv_m;

    static const int RENDER_TEXTURE_DEPTH = 16;
    static const byte TEXTURE_STREAM_EVENT = 1;

    std::shared_ptr<Texture2D> renderTexture;
    void* captureCamera;

public:
    TextureStreamer() {
        ApplyPresets();
        InitializeTextureStreaming();
    }

    void Awake() {
        ApplyPresets();
        InitializeTextureStreaming();
    }

    void StartStreaming() {
        if (PhotonNetwork::IsConnected) {
            std::thread streamThread(&TextureStreamer::CaptureAndSendTexture, this);
            streamThread.detach();
        }
    }

    void StopStreaming() {
        {
            std::lock_guard<std::mutex> lk(cv_m);
            isStreaming = false;
        }
        cv.notify_one();
    }

    void OnDestroy() {
        isStreaming = false;
    }

    void CaptureAndSendTexture() {
        auto waitDuration = std::chrono::milliseconds(static_cast<int>(waitTime * 1000));

        while (isStreaming) {
            auto texture = texturePool.GetTexture(textureWidth, textureHeight);

            try {
                auto pixels = texture->ReadPixels(0, 0, texture->width, texture->height);
                auto jpegData = texture->EncodeToJPG(compression);

                PhotonNetwork::RaiseEvent(TEXTURE_STREAM_EVENT, jpegData);
            }
            catch (const std::exception& e) {
                std::cerr << "An error occurred: " << e.what() << std::endl;
            }

            std::unique_lock<std::mutex> lk(cv_m);
            if (cv.wait_for(lk, waitDuration, [this] { return !this->isStreaming.load(); })) {
                break;
            }
        }
    }

    void ApplyPresets() {
        textureWidth = static_cast<int>(std::round(desiredResolutionHeight * 16.0 / 9.0));
        textureHeight = desiredResolutionHeight;

        switch (frameRate) {
        case _20FPS: waitTime = 1.0f / 20.0f; break;
        case _24FPS: waitTime = 1.0f / 24.0f; break;
        case _30FPS: waitTime = 1.0f / 30.0f; break;
        case _60FPS: waitTime = 1.0f / 60.0f; break;
        default: assert(false && "Unknown frame rate preset"); break;
        }
    }

    void InitializeTextureStreaming() {
        if (!renderTexture || renderTexture->width != textureWidth || renderTexture->height != textureHeight) {
            renderTexture = std::make_shared<Texture2D>(textureWidth, textureHeight);
        }
    }
};

int main() {
    try {
        TextureStreamer streamer;

        PhotonNetwork::IsConnected = true;
        PhotonNetwork::IsMasterClient = true;

        streamer.Awake();

        streamer.StartStreaming();

        std::cout << "Streaming started. Press Enter to stop...\n";
        std::cin.get();

        streamer.StopStreaming();
        streamer.OnDestroy();

        std::cout << "Streaming stopped.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << '\n';
        return 1;
    }
    return 0;
}