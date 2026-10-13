// ============================================================================
// efekt.h - parcaciklar, toast bildirimleri, buton ve ortak cizim yardimcilari
// ============================================================================

// ---------------------------------------------------------------------------
// PARCACIK SISTEMI (alim/satim/haber anlarinda "juicy" efekt)
// ---------------------------------------------------------------------------
struct Parcacik {
    Vector2 pos;
    Vector2 hiz;
    float omur, maxOmur;
    float boyut;
    Color renk;
};

struct ParcacikSistemi {
    vector<Parcacik> liste;

    void Patlat(Vector2 merkez, Color renk, int adet = 18, float hiz = 220.0f) {
        if (liste.size() > 600) return; // basiz testlerde sinirsiz birikmesin
        for (int i = 0; i < adet; i++) {
            float aci = (float)GetRandomValue(0, 360) * DEG2RAD;
            float s = hiz * (0.4f + (float)GetRandomValue(0, 100) / 100.0f);
            Parcacik p;
            p.pos = merkez;
            p.hiz = {cosf(aci) * s, sinf(aci) * s - 60.0f};
            p.maxOmur = 0.5f + (float)GetRandomValue(0, 60) / 100.0f;
            p.omur = p.maxOmur;
            p.boyut = 2.0f + (float)GetRandomValue(0, 30) / 10.0f;
            p.renk = renk;
            liste.push_back(p);
        }
    }

    void Guncelle(float dt) {
        for (auto& p : liste) {
            p.pos.x += p.hiz.x * dt;
            p.pos.y += p.hiz.y * dt;
            p.hiz.y += 260.0f * dt; // yercekimi
            p.omur -= dt;
        }
        liste.erase(remove_if(liste.begin(), liste.end(),
                              [](const Parcacik& p) { return p.omur <= 0.0f; }),
                    liste.end());
    }

    void Ciz() {
        for (auto& p : liste) {
            float oran = p.omur / p.maxOmur;
            DrawCircleV(p.pos, p.boyut * oran, Fade(p.renk, oran));
        }
    }
};

// ---------------------------------------------------------------------------
// TOAST (BILDIRIM) SISTEMI
// ---------------------------------------------------------------------------
struct Toast {
    string baslik;
    string govde;
    Color renk;
    float omur;
    float maxOmur;
    AnimFloat kaymaX; // ekrana sagdan kayarak girer
};

struct ToastSistemi {
    vector<Toast> liste;

    void Ekle(const string& baslik, const string& govde, Color renk, float sure = 4.0f) {
        Toast t;
        t.baslik = baslik;
        t.govde = govde;
        t.renk = renk;
        t.maxOmur = sure;
        t.omur = sure;
        t.kaymaX.deger = 340.0f; // baslangicta ekran disinda (sagda)
        t.kaymaX.hiz = 8.0f;
        t.kaymaX.Ayarla(0.0f);
        liste.push_back(t);
        if (liste.size() > 4) liste.erase(liste.begin());
    }

    void Guncelle(float dt) {
        for (auto& t : liste) {
            t.kaymaX.Guncelle(dt);
            t.omur -= dt;
            if (t.omur < 0.6f) t.kaymaX.Ayarla(340.0f); // cikista tekrar kaysin
        }
        liste.erase(remove_if(liste.begin(), liste.end(),
                              [](const Toast& t) { return t.omur <= 0.0f; }),
                    liste.end());
    }

    // Govde metni eskiden TEK satirdi ve uzun kuresel haberler kutudan (ve
    // ekrandan) tasiyordu; artik kutu genisligine sarilir, kutu boyu satir
    // sayisina gore buyur (en fazla 3 satir).
    void Ciz(int ekranGenislik) {
        const float kutuW = 300.0f;
        const int govdeW = (int)kutuW - 18 - 14;
        float y = 118.0f;
        for (auto& t : liste) {
            float alpha = Clamp(t.omur / 0.5f, 0.0f, 1.0f);
            float x = (float)ekranGenislik - 320.0f + t.kaymaX.deger;
            int govdeYuk = SariliYukseklik(t.govde, 14, govdeW, 3, 17);
            if (govdeYuk < 17) govdeYuk = 17;
            Rectangle kutu = {x, y, kutuW, 49.0f + (float)govdeYuk};
            DrawRectangleRounded({kutu.x + 3, kutu.y + 4, kutu.width, kutu.height}, 0.18f, 8,
                                 Fade(BLACK, 0.35f * alpha));
            DrawRectangleRounded(kutu, 0.18f, 8, Fade(RENK_PANEL_ACIK, alpha));
            DrawRectangleRounded({kutu.x, kutu.y, 6.0f, kutu.height}, 0.9f, 4, Fade(t.renk, alpha));
            const bool kayiyor = t.kaymaX.deger > 1.0f; // ekrana girerken/cikarken bilerek disarida
            if (kayiyor) TestKaydiDurdur();
            YaziCiz(t.baslik.c_str(), (int)kutu.x + 18, (int)kutu.y + 10, 16, Fade(t.renk, alpha));
            YaziCizSarili(t.govde, (int)kutu.x + 18, (int)kutu.y + 33, 14, govdeW, Fade(RENK_METIN, alpha), 3, 17);
            if (kayiyor) TestKaydiSurdur();
            y += kutu.height + 8.0f;
        }
    }
};

// ---------------------------------------------------------------------------
// BUTON (hover animasyonlu)
// ---------------------------------------------------------------------------
struct Buton {
    Rectangle rect = {0, 0, 0, 0};
    string etiket;
    Color renk = RENK_KENARLIK;
    AnimFloat olcek;
    bool aktif = true;

    Buton() { olcek.deger = 1.0f; olcek.hedef = 1.0f; olcek.hiz = 14.0f; }

    bool Guncelle(float dt, Vector2 fare, bool tiklandi) {
        bool ustunde = aktif && CheckCollisionPointRec(fare, rect);
        olcek.Ayarla(ustunde ? 1.06f : 1.0f);
        olcek.Guncelle(dt);
        return aktif && ustunde && tiklandi;
    }

    void Ciz(int fontBoyu = 16) const {
        float cx = rect.x + rect.width / 2.0f;
        float cy = rect.y + rect.height / 2.0f;
        float w = rect.width * olcek.deger;
        float h = rect.height * olcek.deger;
        Rectangle r = {cx - w / 2.0f, cy - h / 2.0f, w, h};
        Color c = aktif ? renk : Fade(renk, 0.4f);
        DrawRectangleRounded({r.x + 2, r.y + 3, r.width, r.height}, 0.3f, 8, Fade(BLACK, 0.35f));
        DrawRectangleRounded(r, 0.3f, 8, c);
        int tw = YaziOlc(etiket.c_str(), fontBoyu);
#ifdef BORSA_TEST
        TestKutuEkle(rect, etiket, 1);
        if (tw > (int)rect.width - 6) TestKutuEkle(rect, etiket, 3); // etiket butona sigmiyor
        g_testButonIci = true;
#endif
        YaziCiz(etiket.c_str(), (int)(cx - tw / 2.0f), (int)(cy - fontBoyu / 2.0f), fontBoyu,
                aktif ? WHITE : Fade(WHITE, 0.55f));
#ifdef BORSA_TEST
        g_testButonIci = false;
#endif
    }
};

// ---------------------------------------------------------------------------
// ORTAK CIZIM YARDIMCILARI
// ---------------------------------------------------------------------------
static void GlowCizgiCiz(const vector<Vector2>& noktalar, Color renk, float kalinlik = 2.5f) {
    // Basit "glow": altta soluk-kalin, ustte parlak-ince cizgi
    for (size_t i = 0; i + 1 < noktalar.size(); i++)
        DrawLineEx(noktalar[i], noktalar[i + 1], kalinlik * 3.0f, Fade(renk, 0.12f));
    for (size_t i = 0; i + 1 < noktalar.size(); i++)
        DrawLineEx(noktalar[i], noktalar[i + 1], kalinlik, renk);
}

// Kenarlikli panel (iki katmanli teknikle; rounded-rect outline API'sine guvenmez)
static void PanelCiz(Rectangle r, float yuvarlaklik, Color icRenk, Color kenarRenk) {
    DrawRectangleRounded({r.x - 2, r.y - 2, r.width + 4, r.height + 4}, yuvarlaklik, 10, kenarRenk);
    DrawRectangleRounded(r, yuvarlaklik, 10, icRenk);
#ifdef BORSA_TEST
    TestKutuEkle(r, "panel", 2);
#endif
}

// Modal pencere arka plani: tum sahneyi karartir ve yeni bir "katman" baslatir
// (yerlesim denetcisi, karartmanin altinda kalan yazilari ustteki pencereyle
// cakisiyor saymasin diye).
static void ModalKarart(float alpha) {
    DrawRectangle(0, 0, PENCERE_GENISLIK, PENCERE_YUKSEKLIK, Fade(BLACK, alpha));
    KatmanBaslat();
}

// Kesikli yatay cizgi (grafikte emir seviyeleri icin).
static void KesikliYatayCizgi(float x0, float x1, float y, Color renk, float parca = 6.0f, float bosluk = 5.0f) {
    for (float x = x0; x < x1; x += parca + bosluk)
        DrawLineEx({x, y}, {fminf(x + parca, x1), y}, 1.5f, renk);
}

static void MiniSparklineCiz(Rectangle alan, const deque<float>& gecmis, Color renk) {
    if (gecmis.size() < 2) return;
    int n = (int)gecmis.size();
    int baslangic = max(0, n - SPARKLINE_UZUNLUK);
    float minV = 1e9f, maxV = -1e9f;
    for (int i = baslangic; i < n; i++) {
        minV = fminf(minV, gecmis[i]);
        maxV = fmaxf(maxV, gecmis[i]);
    }
    if (maxV - minV < 0.01f) { minV -= 1.0f; maxV += 1.0f; }

    vector<Vector2> noktalar;
    int adet = n - baslangic;
    for (int i = 0; i < adet; i++) {
        float t = (adet <= 1) ? 0.0f : (float)i / (float)(adet - 1);
        float x = alan.x + t * alan.width;
        float degerOran = (gecmis[baslangic + i] - minV) / (maxV - minV);
        float y = alan.y + alan.height - degerOran * alan.height;
        noktalar.push_back({x, y});
    }
    GlowCizgiCiz(noktalar, renk, 1.6f);
}
