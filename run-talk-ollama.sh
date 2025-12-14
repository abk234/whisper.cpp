#!/bin/bash

# Startup script for talk-ollama
# This version uses Ollama API instead of loading a local GGUF model

# Check if Ollama is running
if ! curl -s http://localhost:11434/api/tags >/dev/null 2>&1; then
    echo "Error: Ollama is not running!"
    echo "Please start Ollama first:"
    echo "  - On macOS: Open the Ollama app or run 'ollama serve'"
    echo ""
    exit 1
fi

# List available Ollama models
echo "Available Ollama models:"
ollama list | tail -n +2 | awk '{print "  - " $1}'
echo ""

# Default model (you can change this)
MODEL="${1:-llama3.2}"

echo "Starting Talk-Ollama..."
echo ""
echo "Configuration:"
echo "  - Whisper model: models/ggml-base.en.bin"
echo "  - Ollama model:  $MODEL"
echo "  - TTS:           macOS 'say' command"
echo ""
echo "Press Ctrl+C to stop"
echo ""
echo "Speak now! The AI will respond using your voice input..."
echo ""

./build/bin/whisper-talk-ollama \
  -mw ./models/ggml-base.en.bin \
  -mo "$MODEL" \
  -p "User" \
  -bn "AI Assistant" \
  -t 4 \
  -vms 8000 \
  --vad-thold 0.5 \
  -sp "You are a helpful AI assistant. Keep your responses concise and conversational, limited to 2-3 sentences."

echo ""
echo "Talk-Ollama stopped."
