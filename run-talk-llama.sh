#!/bin/bash

# Simple script to run talk-llama with sensible defaults

echo "Starting Talk-LLaMA..."
echo ""
echo "This will use:"
echo "  - Whisper: models/ggml-base.en.bin"
echo "  - LLaMA: models/llama/llama-3.2-1b-instruct-q4_k_m.gguf"
echo "  - TTS: macOS 'say' command"
echo ""
echo "Press Ctrl+C to stop"
echo ""

./build/bin/whisper-talk-llama \
  -mw ./models/ggml-base.en.bin \
  -ml ./models/llama/llama-3.2-1b-instruct-q4_k_m.gguf \
  -p "User" \
  -bn "AI Assistant" \
  -t 4 \
  -vms 8000 \
  --vad-thold 0.5

echo ""
echo "Talk-LLaMA stopped."
