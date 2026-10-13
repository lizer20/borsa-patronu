// ============================================================================
// platform.h - masaustu / web farklarini tek yerde toplar
// ============================================================================
// Oyunun geri kalani "dosya" ya da "tarayici" bilmez; sadece DepoYaz/DepoOku
// cagirir. Masaustunde .exe'nin YANINDAKI dosyaya, web surumunde tarayicinin
// localStorage alanina yazilir.

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#define BORSA_WEB 1
#else
#define BORSA_WEB 0
#endif

#if BORSA_WEB
EM_JS_DEPS(borsa_depo_bagimliliklari, "$UTF8ToString,$stringToUTF8,$lengthBytesUTF8");

EM_JS(int, js_depo_yaz, (const char* anahtar, const char* veri), {
    try {
        localStorage.setItem(UTF8ToString(anahtar), UTF8ToString(veri));
        return 1;
    } catch (e) {
        return 0;
    }
});

// Donen isaretci malloc ile ayrilir; cagiran free() etmelidir. Anahtar yoksa 0.
EM_JS(char*, js_depo_oku, (const char* anahtar), {
    try {
        var v = localStorage.getItem(UTF8ToString(anahtar));
        if (v === null) return 0;
        var n = lengthBytesUTF8(v) + 1;
        var p = _malloc(n);
        stringToUTF8(v, p, n);
        return p;
    } catch (e) {
        return 0;
    }
});

EM_JS(int, js_tarayici_turkce, (), {
    var l = (navigator.language || navigator.userLanguage || "").toLowerCase();
    return l.indexOf("tr") === 0 ? 1 : 0;
});

static string DepoAnahtari(const string& ad) { return "borsa_patronu:" + ad; }

static bool DepoYaz(const string& ad, const string& icerik) {
    return js_depo_yaz(DepoAnahtari(ad).c_str(), icerik.c_str()) != 0;
}

static bool DepoOku(const string& ad, string& icerik) {
    char* p = js_depo_oku(DepoAnahtari(ad).c_str());
    if (p == nullptr) return false;
    icerik = p;
    free(p);
    return true;
}

static bool DepoVarMi(const string& ad) {
    string bos;
    return DepoOku(ad, bos);
}

#else // ---------------------------- MASAUSTU ----------------------------

// Dosyalari .exe'nin YANINA yaziyoruz, "calisma dizinine" DEGIL: oyun bir
// kisayoldan ya da itch.io uygulamasindan baslatildiginda calisma dizini
// bambaska bir klasor olabiliyor ve kayit "kaybolmus" gorunuyordu.
static string DepoYolu(const string& ad) {
    const char* dizin = GetApplicationDirectory();
    return (dizin != nullptr ? string(dizin) : string()) + ad;
}

static bool DepoYaz(const string& ad, const string& icerik) {
    ofstream f(DepoYolu(ad), ios::trunc);
    if (!f.is_open()) return false;
    f << icerik;
    return f.good();
}

static bool DepoOku(const string& ad, string& icerik) {
    ifstream f(DepoYolu(ad));
    if (!f.is_open()) return false;
    ostringstream ss;
    ss << f.rdbuf();
    icerik = ss.str();
    return true;
}

static bool DepoVarMi(const string& ad) {
    ifstream f(DepoYolu(ad));
    return f.good();
}

#endif

// Ilk acilista dil secimi icin: isletim sistemi / tarayici Turkce mi?
// (windows.h raylib ile isim cakismasi yaptigi icin - Rectangle, CloseWindow,
// ShowCursor - tek gereken fonksiyonu elle bildiriyoruz.)
#if !BORSA_WEB && defined(_WIN32)
extern "C" __declspec(dllimport) unsigned short __stdcall GetUserDefaultUILanguage(void);
#endif

static bool SistemDiliTurkce() {
#if BORSA_WEB
    return js_tarayici_turkce() != 0;
#elif defined(_WIN32)
    return (GetUserDefaultUILanguage() & 0x3ff) == 0x1f; // 0x1f = LANG_TURKISH
#else
    const char* l = getenv("LANG");
    return l != nullptr && l[0] == 't' && l[1] == 'r';
#endif
}

// Web surumunde "oyundan cik" ve pencere modu secenekleri anlamsizdir (sekmeyi
// kullanici kapatir, tam ekrani sayfa/itch.io yonetir); arayuz bunlari gizler.
static bool PlatformdaCikisVar() { return !BORSA_WEB; }
