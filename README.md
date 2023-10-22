# Texture Streamer for Unity and Unreal Engine

This repository provides scripts for texture streaming over the network using both Unity (with Photon Unity Networking) and Unreal Engine. It captures frames from a designated camera in the scene (or the respective equivalent in Unreal) and sends them to other clients in the room. This process allows for real-time sharing of visual information, crucial for various applications such as remote collaboration or multiplayer games where players need to share their perspectives.

## Features
- Real-time texture streaming from one client to others in a networked environment.
- Adjustable streaming frame rate and compression for performance management.
- Pooling system for textures to reduce memory usage and garbage collection overhead.
- Comprehensive error handling and validation to ensure smooth operation.
- Supports both Unity (with Photon PUN) and Unreal Engine (currently in alpha phase).

## Pre-requisites
Before you begin, ensure you have met the following requirements:

### For Unity:
- You have a working Unity project.
- You have installed Photon Unity Networking (PUN) in your Unity project.

### For Unreal Engine (Alpha Version):
- Please note that the Unreal version is currently in an alpha phase. Some features might be experimental.
- Ensure you have the necessary Unreal Engine setup to compile and run the provided C++ scripts.

## Setup Guides

### Texture Streamer for Unity
[Detailed instructions for Unity setup.]

### Texture Streamer for Unreal Engine (Alpha)
The Unreal Engine version is implemented in C++ and provides a similar functionality but is currently in alpha. Check the provided code above to get a sense of the implementation. As it's in alpha, expect potential updates and improvements over time.

## Important Methods and Classes (Unreal Engine Version)

- **Texture2D**: Simulates a 2D texture, providing methods to read and encode pixel data.
- **TexturePool**: Manages a pool of `Texture2D` objects, offering a thread-safe way to obtain or return textures.
- **PhotonNetwork**: Simulated Photon network class for the example. In a real-world scenario, replace with actual networking logic.
- **TextureStreamer**: The main class that handles the logic for capturing textures and sending them over the network.

## Limitations and Notes

- For Unity: The script is designed to stop functioning if it's not the Master Client or if it's not connected to Photon. Adjust if necessary for your use case.
- For Unreal: The Unreal version is in alpha. Some features might be experimental or subject to change.
- Ensure you have sufficient bandwidth as real-time texture streaming can be data-intensive, especially with higher qualities and frame rates.