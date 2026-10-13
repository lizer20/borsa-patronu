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

// ---- KACMAYAN TIKLAMA ----
// raylib fare tusunu "su an basili mi" diye KARE BASINA bir kez yoklar. Tarayicida
// basma ve birakma olaylari AYNI kare icinde gelirse (cok hizli tik, dusuk kare
// hizi, otomasyon araclari) tik hic gorulmeden kaybolur. Bunu onlemek icin
// mousedown olayini kendimiz yakalayip konumuyla birlikte sakliyoruz; GirdiOku()
// her karede bunu da sorar.
// DOKUNMATIK EKRAN: raylib dokunusu fare tusuna esler, ama onu da kare basina bir
// kez yoklar; yavas bir telefonda hizli bir dokunus (basma + kaldirma ayni karede)
// yine kaybolurdu. Bu yuzden touchstart da ayni kuyruga yazilir. raylib ile kuyruk
// ayni dokunusu AYNI karede bildirir, yani tek tik sayilir.
// Tarayici bir dokunustan sonra "uyumluluk" icin sahte bir mousedown da uretebilir
// (raylib bunu engeller, ama her tarayiciya guvenmiyoruz); onu da sayarsak tek
// dokunus IKI tik olurdu (cift alim!). Bir dokunustan sonraki 800 ms icinde gelen
// mousedown bu yuzden yok sayilir.
EM_JS(void, js_tik_dinleyici_kur, (), {
    Module.borsaTik = -1;
    var c = Module.canvas;
    var sonDokunma = -1e9;
    var kaydet = function (cx, cy) {
        var r = c.getBoundingClientRect();
        if (r.width <= 0 || r.height <= 0) return;
        var x = Math.max(0, Math.min(9999, Math.round((cx - r.left) * (c.width / r.width))));
        var y = Math.max(0, Math.min(9999, Math.round((cy - r.top) * (c.height / r.height))));
        Module.borsaTik = x * 10000 + y;
    };
    c.addEventListener('touchstart', function (e) {
        sonDokunma = performance.now();
        // Yalnizca ILK parmak tik sayilir (ikinci parmak yanlislikla islem yapmasin).
        if (e.touches && e.touches.length === 1 && e.changedTouches && e.changedTouches.length > 0)
            kaydet(e.changedTouches[0].clientX, e.changedTouches[0].clientY);
    }, {capture: true, passive: true});
    c.addEventListener('touchend', function () { sonDokunma = performance.now(); }, {capture: true, passive: true});
    c.addEventListener('mousedown', function (e) {
        if (e.button !== 0) return;
        if (performance.now() - sonDokunma < 800) return;
        kaydet(e.clientX, e.clientY);
    }, true);
});

// Bekleyen bir tik varsa x*10000+y (tuval pikseli) doner ve temizler; yoksa -1.
EM_JS(double, js_tik_al, (), {
    var t = Module.borsaTik;
    Module.borsaTik = -1;
    return (typeof t === 'number') ? t : -1;
});

// ---- TUVAL BOYUTU ----
// Tuvalin (canvas) piksel boyutu = tuvalin SAYFADAKI kutusu x ekranin piksel
// yogunlugu. Boylece yuksek yogunluklu ekranlarda (telefon, Retina) yazi bulanik
// olmaz. Kutunun kendisi olculur (pencere boyutu degil): telefonda adres cubugu
// gorunup kaybolurken ikisi kisa sureligine farkli olabilir ve goruntu esnerdi.
// Yogunluk 2 ile sinirli: 3x bir telefonda tuval gereksiz buyur, oyun yavaslar.
// Donus: genislik*10000 + yukseklik. UygulamaKare bunu her karede sorar ve tuval
// farkliysa SetWindowSize ile uydurur (pencere/telefon yonu degisince de).
EM_JS(double, js_tuval_hedef_boyutu, (), {
    var dpr = Math.min(window.devicePixelRatio || 1, 2);
    var r = Module.canvas.getBoundingClientRect();
    var cw = (r.width > 0) ? r.width : window.innerWidth;
    var ch = (r.height > 0) ? r.height : window.innerHeight;
    var w = Math.max(320, Math.min(4096, Math.round(cw * dpr)));
    var h = Math.max(240, Math.min(4096, Math.round(ch * dpr)));
    return w * 10000 + h;
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
