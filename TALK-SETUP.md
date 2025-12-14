# Talk-LLaMA Setup Guide

This repository now has two voice chat applications configured and ready to use!

## Quick Start

### Option 1: Talk-LLaMA (Local GGUF Model)
Uses a local Llama-3.2-1B model file - completely offline, no API needed.

```bash
./run-talk-llama.sh
```

### Option 2: Talk-Ollama (Uses Ollama API)
Uses Ollama with any model you have installed (llama3.2, gemma2, qwen, etc.)

```bash
# Make sure Ollama is running first
ollama serve  # or just open the Ollama app

# Run with default model (llama3.2)
./run-talk-ollama.sh

# Or specify a different model
./run-talk-ollama.sh gemma2:9b
./run-talk-ollama.sh qwen2.5:7b
```

---

## What's Installed

### Dependencies
- ✅ **CMake** (4.2.1) - Build system
- ✅ **SDL2** (2.32.10) - Microphone audio capture
- ✅ **Ollama** - Local LLM API server
- ✅ **macOS `say`** - Text-to-speech (built-in)

### Models
- ✅ **Whisper base.en** (142 MB) - Speech recognition
- ✅ **Llama-3.2-1B-Instruct** (770 MB) - Fast local chat model
- ✅ **Ollama models** - Your existing models (gemma2:9b, qwen2.5:7b, llama3.2, etc.)

### Executables
- `./build/bin/whisper-cli` - Command-line transcription
- `./build/bin/whisper-talk-llama` - Voice chat with local GGUF model
- `./build/bin/whisper-talk-ollama` - Voice chat with Ollama API

---

## Comparison: Talk-LLaMA vs Talk-Ollama

| Feature | Talk-LLaMA | Talk-Ollama |
|---------|------------|-------------|
| **Model Format** | GGUF file | Ollama API |
| **Internet Required** | ❌ No | ❌ No (local API) |
| **Model Switching** | Restart with different file | Change via command line |
| **Model Size** | 770 MB (Llama-3.2-1B) | Depends on Ollama model |
| **Speed** | Very Fast | Fast (depends on model) |
| **Memory** | Lower | Varies by model |
| **Best For** | Offline use, embedded systems | Flexibility, larger models |

---

## Usage Examples

### Basic Usage

**Talk-LLaMA:**
```bash
./run-talk-llama.sh
# Just speak! The AI will respond.
```

**Talk-Ollama:**
```bash
./run-talk-ollama.sh
# Or with a specific model:
./run-talk-ollama.sh qwen2.5:7b
```

### Advanced: With Wake Command

Only respond when you say "Hey Assistant":

**Talk-LLaMA:**
```bash
./build/bin/whisper-talk-llama \
  -mw ./models/ggml-base.en.bin \
  -ml ./models/llama/llama-3.2-1b-instruct-q4_k_m.gguf \
  -w "Hey Assistant" \
  -ho "Yes?" \
  -p "User" \
  -bn "Assistant"
```

**Talk-Ollama:**
```bash
./build/bin/whisper-talk-ollama \
  -mw ./models/ggml-base.en.bin \
  -mo "llama3.2" \
  -w "Hey Computer" \
  -ho "I'm listening" \
  -p "User" \
  -bn "Computer"
```

### Save Conversation History

**Talk-LLaMA** (session file):
```bash
./build/bin/whisper-talk-llama \
  -mw ./models/ggml-base.en.bin \
  -ml ./models/llama/llama-3.2-1b-instruct-q4_k_m.gguf \
  --session ./my-conversation.bin
```

**Talk-Ollama** (maintains last 5 exchanges in memory automatically)

---

## Command-Line Options

### Common Options (Both Applications)

```
-mw FILE   Whisper model file (default: models/ggml-base.en.bin)
-p NAME    Your name (default: "User" or "Georgi")
-bn NAME   Bot's name (default: "AI" or "LLaMA")
-t N       CPU threads (default: 4)
-vms N     Voice duration in ms (default: 10000)
-w TEXT    Wake command (optional, e.g., "Hey Assistant")
-ho TEXT   Response after wake command (e.g., "Yes?")
-l LANG    Language (default: "en")
--vad-thold N   Voice detection threshold (default: 0.6)
-c ID      Microphone device ID (default: -1 for default mic)
-ng        Disable GPU acceleration
```

### Talk-LLaMA Specific

```
-ml FILE   LLaMA model file (GGUF format)
--session FILE   Save/load conversation state
-ngl N     GPU layers (default: 999 for full GPU)
--temp N   Temperature (default: 0.30)
--top-p N  Top-p sampling (default: 0.80)
```

### Talk-Ollama Specific

```
-mo NAME   Ollama model name (default: "llama3.2")
-sp TEXT   System prompt for the AI
```

---

## Troubleshooting

### "Audio init failed"
- Check microphone permissions in System Settings > Privacy & Security > Microphone
- Try specifying a different capture device: `-c 0` or `-c 1`

### "Ollama is not running" (Talk-Ollama only)
```bash
# Start Ollama
ollama serve
# Or just open the Ollama app
```

### "No response from AI"
- Speak clearly and wait for processing
- Check VAD threshold: lower it with `--vad-thold 0.4` if not detecting voice
- Increase voice duration: `-vms 12000` for longer recording

### "AI response is too verbose"
- Adjust system prompt (Talk-Ollama): `-sp "Be very concise. One sentence only."`
- Use lower temperature (Talk-LLaMA): `--temp 0.1`

### List available microphones
```bash
# macOS
system_profiler SPAudioDataType

# Or use the tool itself - it will show devices if capture fails
```

---

## Performance Tips

1. **Use GPU acceleration** (enabled by default on M2 Max)
   - Talk-LLaMA: Automatically uses Metal
   - Talk-Ollama: Ollama uses GPU automatically

2. **Adjust voice duration** based on speaking speed:
   - Fast speakers: `-vms 6000` (6 seconds)
   - Normal: `-vms 8000` (8 seconds, default in scripts)
   - Slow/thoughtful: `-vms 12000` (12 seconds)

3. **Choose the right model**:
   - Talk-LLaMA: Already optimized with Llama-3.2-1B
   - Talk-Ollama: Use smaller models for faster response
     - Fast: `llama3.2` (2GB), `qwen2.5:7b` (4.7GB)
     - Quality: `gemma2:9b` (5.4GB), `qwen2.5:14b` (9GB)
     - Slow but smart: `deepseek-r1:32b` (19GB)

---

## Next Steps

### Download More Whisper Models
```bash
# Tiny (75 MB) - fastest, least accurate
./models/download-ggml-model.sh tiny.en

# Small (466 MB) - good balance
./models/download-ggml-model.sh small.en

# Medium (1.5 GB) - better accuracy
./models/download-ggml-model.sh medium.en
```

### Download More LLaMA Models for Talk-LLaMA

Visit [Hugging Face GGUF models](https://huggingface.co/models?sort=trending&search=gguf) and download to `./models/llama/`

Recommended:
- [Qwen2.5-7B-Instruct](https://huggingface.co/Qwen/Qwen2.5-7B-Instruct-GGUF)
- [Gemma-2-9B](https://huggingface.co/bartowski/gemma-2-9b-it-GGUF)
- [Phi-3-mini](https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf)

### Install More Ollama Models

```bash
# Fast models
ollama pull llama3.2
ollama pull qwen2.5:7b

# Balanced
ollama pull gemma2:9b
ollama pull mistral

# Advanced reasoning
ollama pull deepseek-r1:7b
ollama pull qwen2.5:14b
```

---

## File Locations

```
whisper.cpp/
├── build/bin/
│   ├── whisper-cli              # Basic transcription
│   ├── whisper-talk-llama       # Voice chat (GGUF)
│   └── whisper-talk-ollama      # Voice chat (Ollama)
├── models/
│   ├── ggml-base.en.bin         # Whisper model
│   └── llama/
│       └── llama-3.2-1b-instruct-q4_k_m.gguf
├── examples/talk-llama/
│   ├── speak                    # TTS script
│   ├── talk-llama.cpp          # Original source
│   └── talk-ollama.cpp         # Ollama version source
├── run-talk-llama.sh           # Quick start script (GGUF)
└── run-talk-ollama.sh          # Quick start script (Ollama)
```

---

## Credits

- **whisper.cpp**: https://github.com/ggerganov/whisper.cpp
- **Ollama**: https://ollama.com
- **Meta Llama**: https://llama.meta.com

---

Enjoy your AI voice assistant! 🎤🤖
