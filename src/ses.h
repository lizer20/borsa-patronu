// ============================================================================
// ses.h - prosedurel ses efektleri ve MUZIK (harici .wav/.mp3 dosyasi YOK)
// ============================================================================
// Butun sesler sinus dalgasi + kisa fade zarfi ile kodda uretilir; proje bu
// sayede "asset-free" kalir. Yeni ses eklerken ayni yontemi kullan.

static Sound TonUret(const vector<pair<float, float>>& notalar, float sesSeviyesi = 0.32f) {
    const int ornekHizi = 44100;
    int toplamOrnek = 0;
    for (auto& n : notalar) toplamOrnek += (int)(n.second * ornekHizi);
    if (toplamOrnek <= 0) toplamOrnek = 1;

    vector<short> veri(toplamOrnek, 0);
    int yazilan = 0;
    for (auto& nota : notalar) {
        float frekans = nota.first;
        int adet = (int)(nota.second * ornekHizi);
        int fadeOrnek = (int)(0.01f * ornekHizi); // 10ms fade in/out -> "tik" sesini onler
        for (int i = 0; i < adet && yazilan < toplamOrnek; i++, yazilan++) {
            float t = (float)i / ornekHizi;
            float genlik = sesSeviyesi;
            if (i < fadeOrnek) genlik *= (float)i / fadeOrnek;
            if (i > adet - fadeOrnek) genlik *= (float)(adet - i) / fadeOrnek;
            float ornek = sinf(2.0f * PI * frekans * t) * genlik;
            veri[yazilan] = (short)(ornek * 32000.0f);
        }
    }

    Wave dalga{};
    dalga.frameCount = toplamOrnek;
    dalga.sampleRate = ornekHizi;
    dalga.sampleSize = 16;
    dalga.channels = 1;
    dalga.data = veri.data();
    // LoadSoundFromWave verileri kendi ic tamponuna kopyalar; fonksiyon donunce
    // 'veri' vektorunun serbest birakilmasi guvenlidir.
    return LoadSoundFromWave(dalga);
}

// ---------------------------------------------------------------------------
// MUZIK (v2.1) - 24 saniyelik, kendi ustune sarilan sakin bir dongu
// ---------------------------------------------------------------------------
// Dort akor (La minor - Fa - Do - Sol), her biri iki olcu, 80 vurus/dk.
// Uc katman:
//   ped   : akorun uc sesi, yavas girip cikan yumusak bir dolgu
//   bas   : her iki vurusta bir kok ses, cabuk sonen
//   arpej : sekizlik notalarla akor sesleri (ikinci olcude bir oktav yukari)
// Dongunun basi ve sonu sessizlige yakin oldugu icin (ped akor sinirlarinda
// sifira iner) tekrar baslarken "tik" duyulmaz. Hafif bir yanki dongunun sonundan
// basina sarilir, boylece ek yeri de dolar.
static const int MUZIK_ORNEK_HIZI = 22050; // ped agirlikli bir doku icin fazlasiyla yeterli

// Muzigin ham ornekleri (16 bit, tek kanal, MUZIK_ORNEK_HIZI). Oyun bunu bir
// Sound'a yukler; tanitim videosu ayni muzigi ses izine yazar.
static vector<short> MuzikOrnekleri() {
    const int hiz = MUZIK_ORNEK_HIZI;
    const float vurus = 0.75f;   // 80 vurus/dk
    const float akorSuresi = 8.0f * vurus; // iki olcu = 6 sn
    const int akorOrnek = (int)(akorSuresi * hiz);
    const int toplam = 4 * akorOrnek;
    vector<float> tampon(toplam, 0.0f);

    static const float akorlar[4][3] = {
        {110.00f, 130.81f, 164.81f}, // La minor : La2 Do3 Mi3
        {87.31f, 110.00f, 130.81f},  // Fa       : Fa2 La2 Do3
        {130.81f, 164.81f, 196.00f}, // Do       : Do3 Mi3 Sol3
        {98.00f, 123.47f, 146.83f},  // Sol      : Sol2 Si2 Re3
    };
    static const int desen[8] = {0, 1, 2, 1, 0, 2, 1, 2};
    const float ikiPi = 2.0f * PI;

    for (int a = 0; a < 4; a++) {
        const int bas = a * akorOrnek;
        // ---- ped ----
        for (int k = 0; k < 3; k++) {
            const float f = akorlar[a][k];
            for (int i = 0; i < akorOrnek; i++) {
                float t = (float)i / hiz;
                float zarf = fminf(1.0f, t / 1.3f) * fminf(1.0f, (akorSuresi - t) / 1.3f);
                float s = sinf(ikiPi * f * t) + 0.35f * sinf(ikiPi * 2.0f * f * t + 0.5f) + 0.12f * sinf(ikiPi * 3.0f * f * t);
                tampon[bas + i] += s * zarf * 0.050f;
            }
        }
        // ---- bas: 1. ve 3. vuruslar ----
        for (int v = 0; v < 8; v += 2) {
            const int notaBas = bas + (int)(v * vurus * hiz);
            const int notaOrnek = (int)(1.3f * hiz);
            const float f = akorlar[a][0];
            for (int i = 0; i < notaOrnek && notaBas + i < toplam; i++) {
                float t = (float)i / hiz;
                float zarf = fminf(1.0f, t / 0.01f) * expf(-2.4f * t);
                tampon[notaBas + i] += sinf(ikiPi * f * t) * zarf * 0.17f;
            }
        }
        // ---- arpej: sekizlik notalar ----
        for (int e = 0; e < 16; e++) {
            const int notaBas = bas + (int)(e * vurus * 0.5f * hiz);
            const int notaOrnek = (int)(0.55f * hiz);
            const float f = akorlar[a][desen[e % 8]] * (e >= 8 ? 4.0f : 2.0f);
            for (int i = 0; i < notaOrnek && notaBas + i < toplam; i++) {
                float t = (float)i / hiz;
                float zarf = fminf(1.0f, t / 0.006f) * expf(-5.5f * t);
                tampon[notaBas + i] += (sinf(ikiPi * f * t) + 0.2f * sinf(ikiPi * 2.0f * f * t)) * zarf * 0.045f;
            }
        }
    }

    // Yanki (dongu boyunca sarilir): derinlik katar ve ek yerini yumusatir.
    const int gecikme = (int)(0.28f * hiz);
    vector<float> yankili(toplam);
    for (int i = 0; i < toplam; i++) {
        int j = (i - gecikme + toplam) % toplam;
        int j2 = (i - 2 * gecikme + 2 * toplam) % toplam;
        yankili[i] = tampon[i] + 0.30f * tampon[j] + 0.12f * tampon[j2];
    }

    vector<short> veri(toplam);
    for (int i = 0; i < toplam; i++) {
        float s = Clamp(yankili[i], -1.0f, 1.0f);
        veri[i] = (short)(s * 30000.0f);
    }
    return veri;
}

static Sound MuzikUret() {
    vector<short> veri = MuzikOrnekleri();
    Wave dalga{};
    dalga.frameCount = (unsigned int)veri.size();
    dalga.sampleRate = MUZIK_ORNEK_HIZI;
    dalga.sampleSize = 16;
    dalga.channels = 1;
    dalga.data = veri.data();
    return LoadSoundFromWave(dalga);
}

struct SesEfektleri {
    Sound alim, satimKar, satimZarar, haberDuyuru, haberEtki, rutbeAtlama, hataUyari, tikla;
    Sound emirGerceklesti, basarim, alarm;
    Sound muzik;
    bool yuklendi = false;

    void Yukle() {
        // Ses cihazi gercekten hazir degilse (hoparlor yok, surucu sorunu vb.)
        // hic ses uretmeyi denemiyoruz; boylece PlaySound/UnloadSound gecersiz
        // bir Sound uzerinde asla cagrilmaz ve oyun cokmez, sadece sessiz calisir.
        // Bu kasitli bir savunma - silme.
        if (!IsAudioDeviceReady()) {
            yuklendi = false;
            return;
        }
        alim = TonUret({{740.0f, 0.05f}, {988.0f, 0.06f}});
        satimKar = TonUret({{523.0f, 0.05f}, {659.0f, 0.05f}, {880.0f, 0.09f}});
        satimZarar = TonUret({{392.0f, 0.05f}, {330.0f, 0.09f}});
        haberDuyuru = TonUret({{523.0f, 0.05f}, {523.0f, 0.05f}});
        haberEtki = TonUret({{440.0f, 0.07f}});
        rutbeAtlama = TonUret({{523.0f, 0.08f}, {659.0f, 0.08f}, {784.0f, 0.08f}, {1046.0f, 0.16f}});
        hataUyari = TonUret({{200.0f, 0.12f}});
        tikla = TonUret({{900.0f, 0.02f}});
        emirGerceklesti = TonUret({{660.0f, 0.04f}, {660.0f, 0.04f}, {990.0f, 0.08f}});
        basarim = TonUret({{784.0f, 0.07f}, {988.0f, 0.07f}, {1175.0f, 0.07f}, {1568.0f, 0.18f}});
        alarm = TonUret({{330.0f, 0.12f}, {247.0f, 0.12f}, {330.0f, 0.12f}, {247.0f, 0.18f}});
        muzik = MuzikUret();
        SetSoundVolume(muzik, 0.55f); // efektlerin altinda, arka planda
        yuklendi = true;
    }
    void Bosalt() {
        if (!yuklendi) return;
        UnloadSound(alim);
        UnloadSound(satimKar);
        UnloadSound(satimZarar);
        UnloadSound(haberDuyuru);
        UnloadSound(haberEtki);
        UnloadSound(rutbeAtlama);
        UnloadSound(hataUyari);
        UnloadSound(tikla);
        UnloadSound(emirGerceklesti);
        UnloadSound(basarim);
        UnloadSound(alarm);
        UnloadSound(muzik);
        yuklendi = false;
    }
};

static bool sesAcik = true;
// Cal() icinde "yuklendi" bayragini kontrol edebilmek icin. Pencere/ses cihazi
// olmadan calisan testlerde (denge testi) bu isaretci bos kalir ve Cal() hicbir
// sey yapmaz - simulasyon bu sayede tamamen "basiz" (headless) calisabilir.
static SesEfektleri* g_sesler = nullptr;
static void Cal(Sound& s) {
    if (sesAcik && g_sesler && g_sesler->yuklendi) PlaySound(s);
}

// Her karede: muzik calmali mi? Bitince bastan baslatir (dongu), kapatilinca
// durdurur. Ses kapaliyken (M tusu / Ses: Kapali) muzik de susar.
static void MuzikGuncelle(bool istenen) {
    if (!g_sesler || !g_sesler->yuklendi) return;
    const bool calmali = istenen && sesAcik;
    const bool caliyor = IsSoundPlaying(g_sesler->muzik);
    if (calmali && !caliyor) PlaySound(g_sesler->muzik);
    else if (!calmali && caliyor) StopSound(g_sesler->muzik);
}
