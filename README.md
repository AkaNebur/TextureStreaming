# Texture Streamer for Unity with Photon PUN

This Unity script enables texture streaming over the network using Photon Unity Networking (PUN). It captures frames from a designated camera in the scene and sends them to other clients in the room. This process allows for real-time sharing of visual information, crucial for various applications such as remote collaboration or multiplayer games where players need to share their perspectives.

## Features
- Real-time texture streaming from one client to others in a Photon PUN room.
- Adjustable streaming frame rate and compression for performance management.
- Pooling system for textures to reduce memory usage and garbage collection overhead.
- Comprehensive error handling and validation to ensure smooth operation.

## Pre-requisites
Before you begin, ensure you have met the following requirements:
- You have a working Unity project.
- You have installed Photon Unity Networking (PUN) in your Unity project.

## How to Set Up Texture Streamer
To add Texture Streamer functionality to your project, follow these steps:

1. **Add the Script:**
    - Download the `TextureStreamer.cs` script.
    - Place the script in your Unity project, typically in the "Scripts" folder.

2. **Set Up the Scene:**
    - Add the script to a GameObject in the scene, or attach it to a prefab.
    - Assign a Camera from your scene to the `captureCamera` field in the inspector of the GameObject where you placed the script.

3. **Configure Streaming Parameters:**
    - Set the `desiredResolutionHeight`, which will affect the width assuming a 16:9 aspect ratio.
    - Choose your `compression` level. Lower values will have better quality but require more bandwidth.
    - Select the `frameRate` preset. You have options ranging from 20 FPS to 60 FPS based on your bandwidth allowances and performance requirements.

4. **Photon Setup:**
    - Ensure you're connected to Photon and have joined a room before attempting to stream textures.
    - Only the Master Client can stream textures. Make adjustments in your room logic if you want all clients to stream their textures.

5. **Run the Scene:**
    - Run your Unity scene. The script will automatically handle the capturing and networking of textures.

## Important Methods

- `InitializeTextureStreaming()`: Sets up the render texture and prepares for streaming.
- `CaptureAndSendTexture()`: Coroutine that captures the texture and sends it to other clients.
- `Dispose()`: Properly disposes of the memory stream and other resources when done.

## Handling Network Events
You might want to react to specific network events, such as receiving a texture. Listen for the `TEXTURE_STREAM_EVENT` in your other Photon scripts.

## Limitations and Notes

- The script is designed to stop functioning if it's not the Master Client or if it's not connected to Photon. Adjust if necessary for your use case.
- Ensure you have sufficient bandwidth as real-time texture streaming can be data-intensive, especially with higher qualities and frame rates.
- This script doesn't directly handle the receiving part of the texture. Implement that in another script that listens for the `TEXTURE_STREAM_EVENT` and processes the data accordingly.
