# Quick Start Guide - Talk-Ollama

## 🚀 Run It Now!

### Step 1: Make sure Ollama is running
```bash
# Check if Ollama is running
curl -s http://localhost:11434/api/tags > /dev/null && echo "✅ Ollama is running" || echo "❌ Start Ollama first"

# If not running, start it:
ollama serve
# Or just open the Ollama app
```

### Step 2: Run Talk-Ollama
```bash
./run-talk-ollama.sh
```

That's it! **Just speak into your microphone** and the AI will respond with voice!

---

## 🎯 Quick Commands

### Use Different Models
```bash
./run-talk-ollama.sh llama3.2       # Fast & small (2GB)
./run-talk-ollama.sh gemma2:9b      # Better quality (5.4GB)
./run-talk-ollama.sh qwen2.5:7b     # Good balance (4.7GB)
```

### See Available Models
```bash
ollama list
```

### Download New Models
```bash
ollama pull llama3.2
ollama pull gemma2:9b
ollama pull qwen2.5:7b
```

---

## 🎤 How to Use

1. **Start the script**: `./run-talk-ollama.sh`
2. **Speak clearly** into your microphone
3. **Wait** for the AI to process and respond
4. **Continue the conversation** naturally
5. **Press Ctrl+C** to stop

---

## ⚙️ Customize

### Make AI more concise
```bash
./build/bin/whisper-talk-ollama \
  -mw ./models/ggml-base.en.bin \
  -mo "llama3.2" \
  -sp "You are a helpful assistant. Always respond in one short sentence."
```

### Add a wake command
```bash
./build/bin/whisper-talk-ollama \
  -mw ./models/ggml-base.en.bin \
  -mo "llama3.2" \
  -w "Hey Assistant" \
  -ho "Yes?"
```

### Adjust voice detection sensitivity
```bash
# More sensitive (picks up quieter speech)
./run-talk-ollama.sh llama3.2 --vad-thold 0.4

# Less sensitive (requires louder speech)
./run-talk-ollama.sh llama3.2 --vad-thold 0.7
```

---

## 🔧 Troubleshooting

### "Ollama is not running"
```bash
ollama serve
```

### "No voice detected"
- Speak louder or closer to the mic
- Lower the threshold: add `--vad-thold 0.4` to the command
- Check microphone permissions in System Settings

### "Response is too slow"
- Use a smaller model: `./run-talk-ollama.sh llama3.2`
- Or use the local GGUF version: `./run-talk-llama.sh`

### "Can't hear the TTS"
- Check system volume
- The TTS uses macOS's built-in `say` command
- Test it: `say "Hello World"`

---

## 📊 Model Recommendations

| Model | Size | Speed | Quality | Best For |
|-------|------|-------|---------|----------|
| llama3.2 | 2GB | ⚡⚡⚡ | ⭐⭐ | Quick responses |
| qwen2.5:7b | 4.7GB | ⚡⚡ | ⭐⭐⭐ | Balanced |
| gemma2:9b | 5.4GB | ⚡⚡ | ⭐⭐⭐⭐ | High quality |
| qwen2.5:14b | 9GB | ⚡ | ⭐⭐⭐⭐⭐ | Best quality |

---

## 📝 Full Documentation

See `TALK-SETUP.md` for complete documentation including:
- Comparison with talk-llama
- All command-line options
- Advanced usage examples
- Performance tuning

---

Enjoy chatting with your local AI! 🤖💬
