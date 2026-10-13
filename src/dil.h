// ============================================================================
// dil.h - iki dilli metin (Turkce / Ingilizce), Turkce harfli font, yazi cizimi
// ============================================================================
// METIN: oyundaki her metin cagri yerinde IKI dilde birden yazilir:
//     M("Kaydet", "Save")
// Arama tablosu / anahtar yok; eksik ceviri DERLEME HATASI olur (ikinci arguman
// verilmezse derlenmez). Veri tablolarinda ayni is icin IkiDil yapisi kullanilir.
//
// FONT: raylib'in varsayilan fontu Latin-1'i kapsar (c, o, u'nun Turkce halleri
// VAR) ama Latin Extended-A'yi kapsamaz; yani  g-breve, s-sedilla, noktasiz i ve
// noktali buyuk I  YOK. Harici bir .ttf gommek yerine (lisans + "asset-free"
// ilkesi) bu 6 glif, varsayilan fontun kendi piksel stiline uyacak sekilde KOD
// ILE cizilip atlasa ekleniyor. Boylece oyun gercek Turkce yazar ve hala tek
// dosyadir. Genislikler taban harflerle AYNI tutuldu (s ile s-sedilla ayni en).

enum class Dil { TR = 0, EN = 1 };
static Dil g_dil = Dil::TR;

static inline const char* M(const char* tr, const char* en) { return g_dil == Dil::EN ? en : tr; }

struct IkiDil {
    const char* tr;
    const char* en;
    const char* al() const { return g_dil == Dil::EN ? en : tr; }
};

// ---------------------------------------------------------------------------
// Test kancasi: BORSA_TEST tanimliyken cizilen her yazi/buton/panelin kutusu
// kaydedilir; test/gorsel_test.cpp bunlari karsilastirip UST USTE BINEN ya da
// panelinden TASAN yazilari otomatik raporlar. Normal derlemede hicbir maliyeti
// yoktur (KatmanBaslat bos bir inline fonksiyondur).
// ---------------------------------------------------------------------------
#ifdef BORSA_TEST
struct TestKutu {
    Rectangle r;
    string metin;
    int katman; // ayni katmandaki kutular birbirine binmemeli
    int tur;    // 0 = yazi, 1 = buton, 2 = panel
};
static vector<TestKutu> g_testKutular;
static int g_testKatman = 0;
static bool g_testKayit = false;
static bool g_testButonIci = false; // buton etiketi cizilirken ayrica kaydetme
static void KatmanBaslat() { g_testKatman++; }
static void TestKutuEkle(Rectangle r, const string& metin, int tur) {
    if (g_testKayit) g_testKutular.push_back({r, metin, g_testKatman, tur});
}
// Veriye bagli / bilerek tasan yazilar (kayan bant, kayarak giren bildirim,
// grafik uzerindeki emir harfleri) denetimden muaf tutulur.
static bool g_testKayitOnceki = false;
static void TestKaydiDurdur() { g_testKayitOnceki = g_testKayit; g_testKayit = false; }
static void TestKaydiSurdur() { g_testKayit = g_testKayitOnceki; }
#else
static inline void KatmanBaslat() {}
static inline void TestKaydiDurdur() {}
static inline void TestKaydiSurdur() {}
#endif

// ---------------------------------------------------------------------------
// FONT
// ---------------------------------------------------------------------------
static Font g_font = {};
static bool g_fontHazir = false;
static Rectangle g_fontRecs[256];
static GlyphInfo g_fontGlyphs[256];

struct EkGlif {
    int kod;
    int genislik;
    const char* satir[10]; // 10 satir, '#' = dolu piksel
};

// Varsayilan fontta buyuk harfler 1-7. satirlarda, kucuk harf govdesi 3-7'de,
// kuyruklar 8-9'da durur. Fontun kendi aksanli buyuk harfleri (O-umlaut vb.)
// aksani 0. satira koyup harfi 2-7'ye sikistiriyor; ayni kural izlendi.
static const EkGlif EK_GLIFLER[] = {
    {0x0131, 1, {".", ".", ".", "#", "#", "#", "#", "#", ".", "."}},                                              // noktasiz i
    {0x0130, 3, {".#.", "...", "###", ".#.", ".#.", ".#.", ".#.", "###", "...", "..."}},                          // noktali buyuk I
    {0x015F, 5, {".....", ".....", ".....", "#####", "#....", "#####", "....#", "#####", "...#.", "..#.."}},      // s-sedilla
    {0x015E, 6, {"......", "######", "#.....", "#.....", "######", ".....#", ".....#", "######", "...#..", "..#..."}}, // S-sedilla
    {0x011F, 5, {"#...#", ".###.", ".....", "#####", "#...#", "#...#", "#...#", "#####", "....#", "#####"}},      // g-breve
    {0x011E, 6, {".#..#.", "..##..", "######", "#.....", "#..###", "#....#", "#....#", "######", "......", "......"}}, // G-breve
};
static const int EK_GLIF_SAYISI = (int)(sizeof(EK_GLIFLER) / sizeof(EK_GLIFLER[0]));

// Pencere acildiktan SONRA cagrilmali (varsayilan font o zaman yuklenir).
static void FontHazirla() {
    Font taban = GetFontDefault();
    if (taban.texture.id == 0 || taban.glyphs == nullptr || taban.recs == nullptr) return;
    if (taban.glyphCount + EK_GLIF_SAYISI > 256) return;
    // Glif gorselleri CPU tarafinda duruyor (raylib varsayilan font icin saklar);
    // yoksa atlasi kuramayiz, varsayilan fontla (ASCII) devam ederiz.
    for (int i = 0; i < taban.glyphCount; i++)
        if (taban.glyphs[i].image.data == nullptr) return;

    Image atlas = GenImageColor(256, 128, BLANK);
    int x = 1, y = 1;
    auto yerAc = [&](int w) -> Rectangle {
        if (x + w + 1 > atlas.width) { x = 1; y += 11; }
        Rectangle r = {(float)x, (float)y, (float)w, 10.0f};
        x += w + 1;
        return r;
    };

    int n = 0;
    for (int i = 0; i < taban.glyphCount; i++, n++) {
        int w = taban.glyphs[i].image.width;
        Rectangle r = yerAc(w);
        ImageDraw(&atlas, taban.glyphs[i].image, {0, 0, (float)w, 10.0f}, r, WHITE);
        g_fontRecs[n] = r;
        g_fontGlyphs[n] = {};
        g_fontGlyphs[n].value = taban.glyphs[i].value;
    }
    for (int e = 0; e < EK_GLIF_SAYISI; e++, n++) {
        const EkGlif& g = EK_GLIFLER[e];
        Rectangle r = yerAc(g.genislik);
        for (int py = 0; py < 10; py++)
            for (int px = 0; px < g.genislik; px++)
                if (g.satir[py][px] == '#') ImageDrawPixel(&atlas, (int)r.x + px, (int)r.y + py, WHITE);
        g_fontRecs[n] = r;
        g_fontGlyphs[n] = {};
        g_fontGlyphs[n].value = g.kod;
    }

    g_font = {};
    g_font.baseSize = taban.baseSize;
    g_font.glyphCount = n;
    g_font.glyphPadding = 0;
    g_font.texture = LoadTextureFromImage(atlas);
    g_font.recs = g_fontRecs;
    g_font.glyphs = g_fontGlyphs;
    UnloadImage(atlas);
    SetTextureFilter(g_font.texture, TEXTURE_FILTER_POINT);
    g_fontHazir = (g_font.texture.id != 0);
}

static void FontBosalt() {
    if (g_fontHazir) UnloadTexture(g_font.texture);
    g_fontHazir = false;
}

// raylib'in DrawText/MeasureText'iyle BIREBIR ayni olcu kurallari (en kucuk boyut
// 10, harf araligi = boyut/10), sadece font farkli. Boylece ASCII metinlerin
// yerlesimi eski surumle piksel piksel ayni kalir.
static int YaziOlc(const char* metin, int boyut) {
    if (metin == nullptr || metin[0] == '\0') return 0;
    if (boyut < 10) boyut = 10;
    Font f = g_fontHazir ? g_font : GetFontDefault();
    if (f.texture.id == 0) return 0;
    return (int)MeasureTextEx(f, metin, (float)boyut, (float)(boyut / 10)).x;
}
static int YaziOlc(const string& metin, int boyut) { return YaziOlc(metin.c_str(), boyut); }

static void YaziCiz(const char* metin, int x, int y, int boyut, Color renk) {
    if (metin == nullptr || metin[0] == '\0') return;
    if (boyut < 10) boyut = 10;
    Font f = g_fontHazir ? g_font : GetFontDefault();
    if (f.texture.id == 0) return;
    DrawTextEx(f, metin, {(float)x, (float)y}, (float)boyut, (float)(boyut / 10), renk);
#ifdef BORSA_TEST
    if (!g_testButonIci && renk.a > 40)
        TestKutuEkle({(float)x, (float)y, (float)YaziOlc(metin, boyut), (float)boyut}, metin, 0);
#endif
}
static void YaziCiz(const string& metin, int x, int y, int boyut, Color renk) {
    YaziCiz(metin.c_str(), x, y, boyut, renk);
}

// Yatayda ortalanmis yazi (merkezX etrafinda).
static void YaziCizOrtali(const string& metin, int merkezX, int y, int boyut, Color renk) {
    YaziCiz(metin.c_str(), merkezX - YaziOlc(metin, boyut) / 2, y, boyut, renk);
}
// Saga yaslanmis yazi (sagX'te biter).
static void YaziCizSaga(const string& metin, int sagX, int y, int boyut, Color renk) {
    YaziCiz(metin.c_str(), sagX - YaziOlc(metin, boyut), y, boyut, renk);
}

// Metni kelime sinirlarindan, verilen genislige sigacak satirlara boler.
static vector<string> SatirlaraBol(const string& metin, int boyut, int maxGenislik) {
    vector<string> satirlar;
    istringstream ss(metin);
    string kelime, satir;
    while (ss >> kelime) {
        string aday = satir.empty() ? kelime : satir + " " + kelime;
        if (!satir.empty() && YaziOlc(aday, boyut) > maxGenislik) {
            satirlar.push_back(satir);
            satir = kelime;
        } else {
            satir = aday;
        }
    }
    if (!satir.empty()) satirlar.push_back(satir);
    return satirlar;
}

// Sarilmis (cok satirli) yazi cizer; kullanilan toplam yuksekligi doner.
// maxSatir > 0 ise fazlasi kesilir ve son satirin sonuna "..." konur.
static int YaziCizSarili(const string& metin, int x, int y, int boyut, int maxGenislik, Color renk,
                         int maxSatir = 0, int satirYuksekligi = 0) {
    if (satirYuksekligi <= 0) satirYuksekligi = boyut + 4;
    vector<string> satirlar = SatirlaraBol(metin, boyut, maxGenislik);
    if (maxSatir > 0 && (int)satirlar.size() > maxSatir) {
        satirlar.resize(maxSatir);
        string& son = satirlar.back();
        while (!son.empty() && YaziOlc(son + "...", boyut) > maxGenislik) son.pop_back();
        son += "...";
    }
    for (size_t i = 0; i < satirlar.size(); i++)
        YaziCiz(satirlar[i].c_str(), x, y + (int)i * satirYuksekligi, boyut, renk);
    return (int)satirlar.size() * satirYuksekligi;
}

static int SariliYukseklik(const string& metin, int boyut, int maxGenislik, int maxSatir = 0,
                           int satirYuksekligi = 0) {
    if (satirYuksekligi <= 0) satirYuksekligi = boyut + 4;
    int n = (int)SatirlaraBol(metin, boyut, maxGenislik).size();
    if (maxSatir > 0 && n > maxSatir) n = maxSatir;
    return n * satirYuksekligi;
}

// ---------------------------------------------------------------------------
// SAYI / PARA / YUZDE BICIMLERI (dile gore)
// ---------------------------------------------------------------------------
// Turkce : 12.345,68   %12,4    +%12,4
// English: 12,345.68   12.4%    +12.4%
static string SayiFormatla(double deger, int ondalik = 2) {
    bool negatif = deger < 0;
    if (negatif) deger = -deger;
    char tampon[64];
    snprintf(tampon, sizeof(tampon), "%.*f", ondalik, deger);
    string s(tampon);
    size_t noktaKonum = s.find('.');
    string tamKisim = (noktaKonum == string::npos) ? s : s.substr(0, noktaKonum);
    string ondalikKisim = (noktaKonum == string::npos) ? "" : s.substr(noktaKonum + 1);

    const char binlik = (g_dil == Dil::EN) ? ',' : '.';
    const char ondalikAyraci = (g_dil == Dil::EN) ? '.' : ',';

    string sonuc;
    int sayac = 0;
    for (int i = (int)tamKisim.size() - 1; i >= 0; i--) {
        sonuc.push_back(tamKisim[i]);
        sayac++;
        if (sayac % 3 == 0 && i != 0) sonuc.push_back(binlik);
    }
    reverse(sonuc.begin(), sonuc.end());
    if (!ondalikKisim.empty()) sonuc += string(1, ondalikAyraci) + ondalikKisim;
    // "-0,00" gibi bir sey yazmayalim
    bool sifir = true;
    for (char c : sonuc)
        if (c >= '1' && c <= '9') { sifir = false; break; }
    return ((negatif && !sifir) ? "-" : "") + sonuc;
}

static string Para(double deger, int ondalik = 2) { return SayiFormatla(deger, ondalik) + " TL"; }

// Isaretli para: "+1.250,00 TL" / "-320,00 TL"
static string ParaIsaretli(double deger, int ondalik = 2) {
    return string(deger >= 0 ? "+" : "") + SayiFormatla(deger, ondalik) + " TL";
}

static string Yuzde(double deger, int ondalik = 1, bool isaretli = false) {
    string isaret;
    if (deger < 0) isaret = "-";
    else if (isaretli) isaret = "+";
    string sayi = SayiFormatla(fabs(deger), ondalik);
    return (g_dil == Dil::EN) ? (isaret + sayi + "%") : (isaret + "%" + sayi);
}
