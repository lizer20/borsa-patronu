// ============================================================================
// ses.h - prosedurel ses efektleri (harici .wav/.mp3 dosyasi YOK)
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

struct SesEfektleri {
    Sound alim, satimKar, satimZarar, haberDuyuru, haberEtki, rutbeAtlama, hataUyari, tikla;
    Sound emirGerceklesti, basarim, alarm;
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
