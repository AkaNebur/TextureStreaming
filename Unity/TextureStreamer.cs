using UnityEngine;
using Photon.Pun;
using System.IO;
using System.IO.Compression;
using Photon.Realtime; 
using ExitGames.Client.Photon;
using System.Collections;
using System.Collections.Generic;
using System;

public class TextureStreamer : MonoBehaviourPunCallbacks {
    public class TexturePool{
        private readonly Queue<Texture2D> availableTextures = new();

        public Texture2D GetTexture(int width, int height){
            if (availableTextures.Count > 0)
                return availableTextures.Dequeue();
            else
                return new Texture2D(width, height, TextureFormat.RGB24, false);
        }

        public void ReturnTexture(Texture2D texture){
            if (texture != null)
                availableTextures.Enqueue(texture);  
        }
    }

    private readonly TexturePool texturePool = new();
    public enum FrameRatePreset {_20FPS, _24FPS, _30FPS, _60FPS, }

    [Header("Setup")]
    public Camera captureCamera;
    public int desiredResolutionHeight = 620; 

    [Range(0, 100)]
    public int compression = 30;
    public FrameRatePreset frameRate;

    private int textureWidth;
    private int textureHeight;

    private float waitTime;

    private const int RENDER_TEXTURE_DEPTH = 16;
    private const byte TEXTURE_STREAM_EVENT = 1;

    private RenderTexture renderTexture;

    private bool presetsApplied = false;
    private bool isInitialized = false;

    private bool isStreaming = true;

    private MemoryStream memoryStream = new();

    private GZipStream zipStream;

    private bool disposed = false;

    private void Awake(){
        if (!ValidateComponents() || !IsEligibleForStreaming()){
            enabled = false;
            return;
        }

        memoryStream = new MemoryStream();
        zipStream = new GZipStream(memoryStream, CompressionMode.Compress, true);

        ApplyPresets();
        InitializeTextureStreaming();
        isInitialized = true;
    }

    private void Start(){
        if (isInitialized){
            StartCoroutine(CaptureAndSendTexture());
        }
    }
    private void OnDestroy(){
        PhotonNetwork.RemoveCallbackTarget(this);
        StopCoroutine(CaptureAndSendTexture());
        Dispose();
    }

    private void InitializeTextureStreaming(){
        if (renderTexture == null || renderTexture.width != textureWidth || renderTexture.height != textureHeight){
            if(renderTexture != null) 
                renderTexture.Release();
            
            renderTexture = new RenderTexture(textureWidth, textureHeight, RENDER_TEXTURE_DEPTH, RenderTextureFormat.ARGB32);
        }
        captureCamera.targetTexture = renderTexture;
    }

    private IEnumerator CaptureAndSendTexture(){
        WaitForSeconds wait = new(waitTime);  

        Texture2D texture2D = texturePool.GetTexture(renderTexture.width, renderTexture.height);

        while (true){
            if(!isStreaming) {
                memoryStream.Close();
                zipStream.Dispose();
                yield break;
            }

            
            RenderTexture prevActive = RenderTexture.active;
            RenderTexture.active = renderTexture;

            texture2D.ReadPixels(new Rect(0, 0, renderTexture.width, renderTexture.height), 0, 0);

            RenderTexture.active = prevActive;

            byte[] textureBytes = texture2D.EncodeToJPG(compression);

            memoryStream.SetLength(0);
            memoryStream.Position = 0;

            using (GZipStream zipStream = new GZipStream(memoryStream, CompressionMode.Compress, true))
                zipStream.Write(textureBytes, 0, textureBytes.Length);
            
            byte[] compressedData = memoryStream.ToArray();

            try {
                PhotonNetwork.RaiseEvent(TEXTURE_STREAM_EVENT, compressedData, new RaiseEventOptions { Receivers = ReceiverGroup.Others }, SendOptions.SendReliable);
            }
            catch (Exception ex) {
                Debug.LogError("Error sending texture data: " + ex.Message);
                yield break;
            }

            Debug.Log($"Sending. Final Size: {compressedData.Length}. JPG Size: {textureBytes.Length}. GZIP Size: {memoryStream.Length}");
            yield return wait; 

        }
    }

    public void Dispose(){
        if (!this.disposed){
            if (zipStream != null){
                zipStream.Dispose();
                zipStream = null;
            }

            if (memoryStream != null){
                memoryStream.Dispose();
                memoryStream = null;
            }

            disposed = true;
        }
    }

    private void ApplyPresets(){
        if (presetsApplied) return;

        textureWidth = (int)(desiredResolutionHeight * 16.0f / 9.0f);
        textureHeight = desiredResolutionHeight;

        switch (frameRate){
            case FrameRatePreset._20FPS:
                waitTime = 1.0f / 20.0f; 
                break;
            case FrameRatePreset._24FPS:
                waitTime = 1.0f / 24.0f; 
                break;
            case FrameRatePreset._30FPS:
                waitTime = 1.0f / 30.0f; 
                break;
            case FrameRatePreset._60FPS:
                waitTime = 1.0f / 60.0f; 
                break;
        }

        presetsApplied = true;
    }

    private bool ValidateComponents(){
        if (captureCamera == null){
            Debug.LogError("Capture Camera not assigned in TextureStreamer.");
            return false;
        }

        if (photonView == null){
            Debug.LogError("PhotonView not found on this component.");
            return false;
        }

        return true;
    }

    private bool IsEligibleForStreaming(){
        if (!PhotonNetwork.IsConnected || !PhotonNetwork.IsMasterClient){
            Debug.LogWarning("TextureStreamer is disabled as it's not connected or not the Master Client.");
            return false;
        }
        return true;
    }
}