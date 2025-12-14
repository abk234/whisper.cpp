// Talk with AI using Ollama API
// Simplified version that uses Ollama instead of local llama.cpp models

#include "common-sdl.h"
#include "common.h"
#include "common-whisper.h"
#include "whisper.h"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <cstdlib>
#include <iomanip>

// Forward declarations for helper functions
static std::string json_escape(const std::string& str);
static std::string unescape_json(const std::string& str);

// Helper function to call Ollama API
std::string call_ollama(const std::string& prompt, const std::string& model = "llama3.2") {
    // Create temp files for the request and response
    std::string request_file = "/tmp/ollama_request.json";
    std::string response_file = "/tmp/ollama_response.json";

    // Create JSON request
    std::ofstream req_file(request_file);
    req_file << "{\n";
    req_file << "  \"model\": \"" << model << "\",\n";
    req_file << "  \"prompt\": " << json_escape(prompt) << ",\n";
    req_file << "  \"stream\": false\n";
    req_file << "}";
    req_file.close();

    // Call Ollama API using curl
    std::string cmd = "curl -s -X POST http://localhost:11434/api/generate "
                      "-H 'Content-Type: application/json' "
                      "-d @" + request_file + " > " + response_file;

    int ret = system(cmd.c_str());
    if (ret != 0) {
        fprintf(stderr, "Failed to call Ollama API\n");
        return "";
    }

    // Read response
    std::ifstream resp_file(response_file);
    std::string response((std::istreambuf_iterator<char>(resp_file)),
                         std::istreambuf_iterator<char>());
    resp_file.close();

    // Parse JSON response to extract the "response" field
    // Simple parsing - look for "response":"..."
    size_t start = response.find("\"response\":\"");
    if (start == std::string::npos) {
        fprintf(stderr, "Could not parse Ollama response\n");
        return "";
    }
    start += 12; // length of "response":""

    size_t end = start;
    while (end < response.length()) {
        if (response[end] == '\"' && (end == 0 || response[end-1] != '\\')) {
            break;
        }
        end++;
    }

    std::string result = response.substr(start, end - start);

    // Unescape the string
    result = unescape_json(result);

    return result;
}

// Helper to escape JSON strings
static std::string json_escape(const std::string& str) {
    std::ostringstream o;
    o << "\"";
    for (char c : str) {
        switch (c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if (c >= 0 && c < 32) {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    o << c;
                }
        }
    }
    o << "\"";
    return o.str();
}

// Helper to unescape JSON strings
static std::string unescape_json(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case 'n': result += '\n'; i++; break;
                case 't': result += '\t'; i++; break;
                case 'r': result += '\r'; i++; break;
                case '\"': result += '\"'; i++; break;
                case '\\': result += '\\'; i++; break;
                default: result += str[i]; break;
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

// command-line parameters
struct whisper_params {
    int32_t n_threads  = std::min(4, (int32_t) std::thread::hardware_concurrency());
    int32_t voice_ms   = 10000;
    int32_t capture_id = -1;
    int32_t max_tokens = 32;
    int32_t audio_ctx  = 0;

    float vad_thold  = 0.6f;
    float freq_thold = 100.0f;

    bool translate      = false;
    bool print_special  = false;
    bool print_energy   = false;
    bool no_timestamps  = true;
    bool use_gpu        = true;
    bool flash_attn     = true;

    std::string person      = "User";
    std::string bot_name    = "AI";
    std::string wake_cmd    = "";
    std::string heard_ok    = "";
    std::string language    = "en";
    std::string model_wsp   = "models/ggml-base.en.bin";
    std::string ollama_model = "llama3.2";
    std::string speak       = "./examples/talk-llama/speak";
    std::string speak_file  = "./examples/talk-llama/to_speak.txt";
    std::string system_prompt = "You are a helpful AI assistant. Keep your responses concise and conversational.";
};

void whisper_print_usage(int argc, char ** argv, const whisper_params & params);

static bool whisper_params_parse(int argc, char ** argv, whisper_params & params) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            whisper_print_usage(argc, argv, params);
            exit(0);
        }
        else if (arg == "-t"   || arg == "--threads")        { params.n_threads      = std::stoi(argv[++i]); }
        else if (arg == "-vms" || arg == "--voice-ms")       { params.voice_ms       = std::stoi(argv[++i]); }
        else if (arg == "-c"   || arg == "--capture")        { params.capture_id     = std::stoi(argv[++i]); }
        else if (arg == "-ac"  || arg == "--audio-ctx")      { params.audio_ctx      = std::stoi(argv[++i]); }
        else if (arg == "-vth" || arg == "--vad-thold")      { params.vad_thold      = std::stof(argv[++i]); }
        else if (arg == "-fth" || arg == "--freq-thold")     { params.freq_thold     = std::stof(argv[++i]); }
        else if (arg == "-tr"  || arg == "--translate")      { params.translate      = true; }
        else if (arg == "-pe"  || arg == "--print-energy")   { params.print_energy   = true; }
        else if (arg == "-ng"  || arg == "--no-gpu")         { params.use_gpu        = false; }
        else if (arg == "-fa"  || arg == "--flash-attn")     { params.flash_attn     = true; }
        else if (arg == "-nfa" || arg == "--no-flash-attn")  { params.flash_attn     = false; }
        else if (arg == "-p"   || arg == "--person")         { params.person         = argv[++i]; }
        else if (arg == "-bn"  || arg == "--bot-name")       { params.bot_name       = argv[++i]; }
        else if (arg == "-w"   || arg == "--wake-command")   { params.wake_cmd       = argv[++i]; }
        else if (arg == "-ho"  || arg == "--heard-ok")       { params.heard_ok       = argv[++i]; }
        else if (arg == "-l"   || arg == "--language")       { params.language       = argv[++i]; }
        else if (arg == "-mw"  || arg == "--model-whisper")  { params.model_wsp      = argv[++i]; }
        else if (arg == "-mo"  || arg == "--model-ollama")   { params.ollama_model   = argv[++i]; }
        else if (arg == "-s"   || arg == "--speak")          { params.speak          = argv[++i]; }
        else if (arg == "-sf"  || arg == "--speak-file")     { params.speak_file     = argv[++i]; }
        else if (arg == "-sp"  || arg == "--system-prompt")  { params.system_prompt  = argv[++i]; }
        else {
            fprintf(stderr, "error: unknown argument: %s\n", arg.c_str());
            whisper_print_usage(argc, argv, params);
            exit(0);
        }
    }

    return true;
}

void whisper_print_usage(int /*argc*/, char ** argv, const whisper_params & params) {
    fprintf(stderr, "\n");
    fprintf(stderr, "usage: %s [options]\n", argv[0]);
    fprintf(stderr, "\n");
    fprintf(stderr, "options:\n");
    fprintf(stderr, "  -h,       --help           [default] show this help message and exit\n");
    fprintf(stderr, "  -t N,     --threads N      [%-7d] number of threads to use during computation\n", params.n_threads);
    fprintf(stderr, "  -vms N,   --voice-ms N     [%-7d] voice duration in milliseconds\n",              params.voice_ms);
    fprintf(stderr, "  -c ID,    --capture ID     [%-7d] capture device ID\n",                         params.capture_id);
    fprintf(stderr, "  -mw FILE, --model-whisper  [%-7s] whisper model file\n",                        params.model_wsp.c_str());
    fprintf(stderr, "  -mo NAME, --model-ollama   [%-7s] ollama model name\n",                         params.ollama_model.c_str());
    fprintf(stderr, "  -p NAME,  --person NAME    [%-7s] person name\n",                                params.person.c_str());
    fprintf(stderr, "  -bn NAME, --bot-name NAME  [%-7s] bot name\n",                                   params.bot_name.c_str());
    fprintf(stderr, "  -l LANG,  --language LANG  [%-7s] spoken language\n",                            params.language.c_str());
    fprintf(stderr, "  -s FILE,  --speak FILE     [%-7s] command for TTS\n",                            params.speak.c_str());
    fprintf(stderr, "  -sp TEXT, --system-prompt  system prompt for the AI\n");
    fprintf(stderr, "  --vad-thold N              [%-7.2f] voice activity detection threshold\n",       params.vad_thold);
    fprintf(stderr, "  --freq-thold N             [%-7.2f] high-pass frequency cutoff\n",               params.freq_thold);
    fprintf(stderr, "  --no-gpu                   disable GPU inference\n");
    fprintf(stderr, "\n");
}

// Function to speak text using TTS (use the one from common-whisper.h)
// Removed duplicate definition - using speak_with_file from common-whisper.h

// Helper function to split text into words
static std::vector<std::string> get_words(const std::string &txt) {
    std::vector<std::string> words;
    std::istringstream iss(txt);
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }
    return words;
}

// Transcribe audio using Whisper
static std::string transcribe(
        whisper_context * ctx,
        const whisper_params & params,
        const std::vector<float> & pcmf32,
        const std::string & prompt_text,
        float & prob,
        int64_t & t_ms) {
    const auto t_start = std::chrono::high_resolution_clock::now();

    prob = 0.0f;
    t_ms = 0;

    std::vector<whisper_token> prompt_tokens;

    whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

    prompt_tokens.resize(1024);
    prompt_tokens.resize(whisper_tokenize(ctx, prompt_text.c_str(), prompt_tokens.data(), prompt_tokens.size()));

    wparams.print_progress   = false;
    wparams.print_special    = params.print_special;
    wparams.print_realtime   = false;
    wparams.print_timestamps = !params.no_timestamps;
    wparams.translate        = params.translate;
    wparams.no_context       = true;
    wparams.single_segment   = true;
    wparams.max_tokens       = params.max_tokens;
    wparams.language         = params.language.c_str();
    wparams.n_threads        = params.n_threads;

    wparams.prompt_tokens    = prompt_tokens.empty() ? nullptr : prompt_tokens.data();
    wparams.prompt_n_tokens  = prompt_tokens.empty() ? 0       : prompt_tokens.size();

    wparams.audio_ctx        = params.audio_ctx;

    if (whisper_full(ctx, wparams, pcmf32.data(), pcmf32.size()) != 0) {
        return "";
    }

    int prob_n = 0;
    std::string result;

    const int n_segments = whisper_full_n_segments(ctx);
    for (int i = 0; i < n_segments; ++i) {
        const char * text = whisper_full_get_segment_text(ctx, i);

        result += text;

        const int n_tokens = whisper_full_n_tokens(ctx, i);
        for (int j = 0; j < n_tokens; ++j) {
            const auto token = whisper_full_get_token_data(ctx, i, j);

            prob += token.p;
            ++prob_n;
        }
    }

    if (prob_n > 0) {
        prob /= prob_n;
    }

    const auto t_end = std::chrono::high_resolution_clock::now();
    t_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();

    return result;
}

int main(int argc, char ** argv) {
    whisper_params params;

    if (whisper_params_parse(argc, argv, params) == false) {
        return 1;
    }

    if (params.language != "auto" && whisper_lang_id(params.language.c_str()) == -1) {
        fprintf(stderr, "error: unknown language '%s'\n", params.language.c_str());
        whisper_print_usage(argc, argv, params);
        exit(0);
    }

    // whisper init
    struct whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu    = params.use_gpu;
    cparams.flash_attn = params.flash_attn;

    struct whisper_context * ctx_wsp = whisper_init_from_file_with_params(params.model_wsp.c_str(), cparams);
    if (!ctx_wsp) {
        fprintf(stderr, "No whisper.cpp model specified. Please provide using -mw <modelfile>\n");
        return 1;
    }

    // print some info about the processing
    {
        fprintf(stderr, "\n");
        if (!whisper_is_multilingual(ctx_wsp)) {
            if (params.language != "en" || params.translate) {
                params.language = "en";
                params.translate = false;
                fprintf(stderr, "%s: WARNING: model is not multilingual, ignoring language and translation options\n", __func__);
            }
        }
        fprintf(stderr, "%s: processing, %d threads, lang = %s, task = %s ...\n",
                __func__,
                params.n_threads,
                params.language.c_str(),
                params.translate ? "translate" : "transcribe");
        fprintf(stderr, "%s: Using Ollama model: %s\n", __func__, params.ollama_model.c_str());
        fprintf(stderr, "\n");
    }

    // init audio
    audio_async audio(30*1000);
    if (!audio.init(params.capture_id, WHISPER_SAMPLE_RATE)) {
        fprintf(stderr, "%s: audio.init() failed!\n", __func__);
        return 1;
    }

    audio.resume();

    bool is_running = true;
    float prob0 = 0.0f;

    std::vector<float> pcmf32_cur;

    const std::string prompt_whisper = "A conversation between " + params.person + " and " + params.bot_name + ".";

    // Conversation history for context
    std::vector<std::string> conversation_history;

    // Wake command setup
    bool use_wake_cmd = !params.wake_cmd.empty();
    std::vector<std::string> wake_cmd_words = get_words(params.wake_cmd);
    int wake_cmd_length = wake_cmd_words.size();

    fprintf(stdout, "\n");
    fprintf(stdout, "%s: Listening ... (press Ctrl+C to stop)\n", __func__);
    if (use_wake_cmd) {
        fprintf(stdout, "%s: Wake command: '%s'\n", __func__, params.wake_cmd.c_str());
    }
    fprintf(stdout, "\n");
    fflush(stdout);

    // main loop
    while (is_running) {
        // handle Ctrl + C
        is_running = sdl_poll_events();
        if (!is_running) {
            break;
        }

        // delay
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        int64_t t_ms = 0;

        {
            audio.get(2000, pcmf32_cur);

            if (::vad_simple(pcmf32_cur, WHISPER_SAMPLE_RATE, 1250, params.vad_thold, params.freq_thold, params.print_energy)) {
                audio.get(params.voice_ms, pcmf32_cur);

                std::string all_heard = ::trim(::transcribe(ctx_wsp, params, pcmf32_cur, prompt_whisper, prob0, t_ms));

                const auto words = get_words(all_heard);

                std::string wake_cmd_heard;
                std::string text_heard;

                for (int i = 0; i < (int) words.size(); ++i) {
                    if (use_wake_cmd && i < wake_cmd_length) {
                        wake_cmd_heard += words[i] + " ";
                    } else {
                        text_heard += words[i] + " ";
                    }
                }

                // check if audio starts with the wake-up command if enabled
                if (use_wake_cmd) {
                    const float sim = similarity(wake_cmd_heard, params.wake_cmd);
                    if ((sim < 0.7f) || (text_heard.empty())) {
                        audio.clear();
                        continue;
                    }
                }

                // optionally give audio feedback
                if (!params.heard_ok.empty()) {
                    speak_with_file(params.speak, params.heard_ok, params.speak_file, 2);
                }

                // Clean up the heard text
                text_heard = std::regex_replace(text_heard, std::regex("\\[.*?\\]"), "");
                text_heard = std::regex_replace(text_heard, std::regex("\\(.*?\\)"), "");
                text_heard = std::regex_replace(text_heard, std::regex("[^a-zA-Z0-9\\.,\\?!\\s\\:\\'\\-]"), "");
                text_heard = text_heard.substr(0, text_heard.find_first_of('\n'));
                text_heard = std::regex_replace(text_heard, std::regex("^\\s+"), "");
                text_heard = std::regex_replace(text_heard, std::regex("\\s+$"), "");

                if (text_heard.empty()) {
                    audio.clear();
                    continue;
                }

                fprintf(stdout, "\n%s%s: %s%s\n", "\033[1m\033[32m", params.person.c_str(), text_heard.c_str(), "\033[0m");
                fflush(stdout);

                // Build conversation context
                std::string full_prompt = params.system_prompt + "\n\n";
                for (const auto & msg : conversation_history) {
                    full_prompt += msg + "\n";
                }
                full_prompt += params.person + ": " + text_heard + "\n";
                full_prompt += params.bot_name + ":";

                // Call Ollama
                fprintf(stdout, "%s%s: ", "\033[1m\033[34m", params.bot_name.c_str());
                fflush(stdout);

                std::string response = call_ollama(full_prompt, params.ollama_model);

                // Clean up response
                response = std::regex_replace(response, std::regex("^\\s+"), "");
                response = std::regex_replace(response, std::regex("\\s+$"), "");

                fprintf(stdout, "%s%s\n\n", response.c_str(), "\033[0m");
                fflush(stdout);

                // Add to conversation history (keep last 5 exchanges)
                conversation_history.push_back(params.person + ": " + text_heard);
                conversation_history.push_back(params.bot_name + ": " + response);
                if (conversation_history.size() > 10) {
                    conversation_history.erase(conversation_history.begin(), conversation_history.begin() + 2);
                }

                // Speak the response
                speak_with_file(params.speak, response, params.speak_file, 2);

                audio.clear();
            }
        }
    }

    audio.pause();
    whisper_free(ctx_wsp);

    return 0;
}
