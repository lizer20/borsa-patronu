// ============================================================================
// gorev.h - GOREVLER: rutbeler arasindaki boslugu dolduran kisa hedefler
// ============================================================================
// Her an TEK bir gorev vardir. Gorevler oyuncunun o anki durumuna gore secilir
// (acik araclar, rutbe, portfoy) ve oyunun mekaniklerini kullanmaya yonlendirir:
// degerleme etiketini okumak, bilancoya hazirlanmak, guvenli limana siginmak...
//
// YENI GOREV TURU EKLEMEK: (1) GorevTuru'na ekle, (2) GorevMetni'ne metnini yaz,
// (3) GorevVer'de ne zaman uygun oldugunu ve hedefini belirle, (4) kosul
// gerceklestigi yerde GorevOlay(o, G_XXX, deger) cagir (ya da GorevGuncelle'de yokla).

static void RutbeVeHedefKontrolEt(Oyun& o); // piyasa_olay.h

enum GorevTuru {
    G_YOK = 0,
    G_KAR_ET,      // tek satista en az X TL kar
    G_HABER,       // iyi bir haber cikinca, fiyata yansimadan once o hisseyi al
    G_CESITLENDIR, // ayni anda K farkli varlikta pozisyon tut
    G_NET_DEGER,   // net degerini X TL'ye cikar
    G_TEMETTU,     // tek temettu odemesinde en az X TL al
    G_BILANCO,     // olumlu gelen bir bilancoyu elinde hisseyle karsila
    G_UCUZ_AL,     // UCUZ / KELEPIR etiketli bir hisseden al
    G_PAHALI_SAT,  // PAHALI / BALON etiketli bir hisseyi karla sat
    G_EMIR,        // bir emrin gerceklessin
    G_ACIGA,       // aciga satistan kar et
    G_LIMAN,       // kotu bir kuresel haber yansirken altin ya da dolar tutuyor ol
    G_DEVRAL,      // bir sirketi devral
    GOREV_TUR_SAYISI
};

// Tutari "yuvarlak" bir sayiya cevirir (iki anlamli basamak): 1.234 -> 1.200.
static float YuvarlakTutar(float x) {
    if (x <= 0.0f) return 0.0f;
    float basamak = powf(10.0f, floorf(log10f(x)) - 1.0f);
    return roundf(x / basamak) * basamak;
}

static string GorevMetni(const Oyun& o) {
    const float h = o.gorevHedef;
    switch (o.gorevTur) {
        case G_KAR_ET:
            return (g_dil == Dil::EN) ? "Earn at least " + Para(h, 0) + " on a single sale"
                                      : "Tek satışta en az " + Para(h, 0) + " kâr et";
        case G_HABER:
            return M("İyi bir haber çıkınca, fiyata yansımadan önce o hisseyi al", "When good news breaks, buy that stock before it hits the price");
        case G_CESITLENDIR:
            return (g_dil == Dil::EN) ? "Hold positions in " + to_string((int)h) + " different assets at once"
                                      : "Aynı anda " + to_string((int)h) + " farklı varlıkta pozisyon tut";
        case G_NET_DEGER:
            return (g_dil == Dil::EN) ? "Bring your net worth to " + Para(h, 0) : "Net değerini " + Para(h, 0) + " yap";
        case G_TEMETTU:
            return (g_dil == Dil::EN) ? "Collect at least " + Para(h, 0) + " in a single dividend"
                                      : "Tek temettü ödemesinde en az " + Para(h, 0) + " al";
        case G_BILANCO:
            return M("Olumlu gelen bir bilançoyu elinde hisseyle karşıla", "Hold a stock through a good earnings report");
        case G_UCUZ_AL:
            return M("UCUZ ya da KELEPİR etiketli bir hisseden al", "Buy a stock tagged CHEAP or BARGAIN");
        case G_PAHALI_SAT:
            return M("PAHALI ya da BALON etiketli bir hisseyi kârla sat", "Sell a stock tagged PRICEY or BUBBLE at a profit");
        case G_EMIR:
            return M("Bir emrin gerçekleşsin (zarar kes / kâr al / limit)", "Have an order fill (stop / target / limit)");
        case G_ACIGA:
            return M("Açığa satıştan kâr et", "Profit from a short sale");
        case G_LIMAN:
            return M("Kötü bir küresel haber yansırken altın ya da dolar tutuyor ol", "Hold gold or dollars when bad global news hits");
        case G_DEVRAL:
            return M("Bir şirketi devral (501 hisse)", "Take over a company (501 shares)");
        default:
            return "";
    }
}

static int GorevKalanGun(const Oyun& o) { return max(0, o.gorevBitisGunu - o.gun + 1); }

// Duruma uygun gorevlerden birini secer ve baslatir.
static void GorevVer(Oyun& o) {
    const float net = o.NetDeger();
    int acikHisse = 0;
    bool ucuzVar = false, pahaliVar = false;
    float enUcuzFiyat = 1e9f;
    for (int i = 0; i < HisseSayisi(); i++) {
        if (!o.HisseAcikMi(i)) continue;
        acikHisse++;
        int sev = DegerlemeSeviyesi(o.hisseler[i]);
        if (sev <= 1) ucuzVar = true;
        if (sev >= 3) pahaliVar = true;
        enUcuzFiyat = fminf(enUcuzFiyat, o.hisseler[i].fiyat);
    }
    int acikVarlik = acikHisse + (o.LimanlarAcik() ? (int)LIMANLAR.size() : 0);
    int cesitHedef = min(acikVarlik, 2 + o.rutbeIndex / 2);

    vector<int> uygun;
    auto ekle = [&](int tur, bool kosul) {
        if (kosul && tur != o.sonGorevTur) uygun.push_back(tur);
    };
    ekle(G_KAR_ET, true);
    ekle(G_HABER, true);
    ekle(G_NET_DEGER, true);
    ekle(G_BILANCO, o.rutbeIndex >= 1);
    ekle(G_CESITLENDIR, acikVarlik >= 2 && o.PozisyonSayisi() < cesitHedef);
    ekle(G_TEMETTU, o.rutbeIndex >= 1);
    ekle(G_UCUZ_AL, o.rutbeIndex >= 2 && ucuzVar);
    ekle(G_PAHALI_SAT, o.rutbeIndex >= 2 && pahaliVar);
    ekle(G_EMIR, o.EmirlerAcik());
    ekle(G_ACIGA, o.AcigaSatisAcik());
    ekle(G_LIMAN, o.LimanlarAcik());
    ekle(G_DEVRAL, o.rutbeIndex >= 2 && o.SahipOlunanSirketSayisi() == 0 && net >= 520.0f * enUcuzFiyat);
    if (uygun.empty()) uygun.push_back(G_KAR_ET);

    o.gorevTur = uygun[g_rngOyuncu.Aralik(0, (int)uygun.size() - 1)];
    o.gorevBitisGunu = o.gun + GOREV_SURE_GUN - 1;
    o.gorevOdul = fmaxf(GOREV_ODUL_TABAN, YuvarlakTutar(net * GOREV_ODUL_ORANI));
    o.gorevHedef = 0.0f;
    switch (o.gorevTur) {
        case G_KAR_ET: o.gorevHedef = fmaxf(100.0f, YuvarlakTutar(net * 0.02f)); break;
        case G_NET_DEGER:
            o.gorevHedef = YuvarlakTutar(net * 1.10f);
            if (o.gorevHedef <= net * 1.04f) o.gorevHedef = YuvarlakTutar(net * 1.18f);
            break;
        case G_CESITLENDIR: o.gorevHedef = (float)cesitHedef; break;
        case G_TEMETTU: o.gorevHedef = fmaxf(10.0f, YuvarlakTutar(net * TEMETTU_ORANI * 0.5f)); break;
        case G_DEVRAL:
            o.gorevOdul *= 3.0f;
            o.gorevBitisGunu = o.gun + GOREV_SURE_GUN * 2 - 1;
            break;
        default: break;
    }
    o.toastlar.Ekle(M("YENİ GÖREV", "NEW MISSION"), GorevMetni(o), RENK_MAVI_VURGU, 6.0f);
}

static void GorevTamamla(Oyun& o) {
    if (o.gorevTur == G_YOK) return;
    o.bakiye += o.gorevOdul;
    o.tamamlananGorev++;
    o.toastlar.Ekle(M("GÖREV TAMAMLANDI!", "MISSION COMPLETE!"), ParaIsaretli(o.gorevOdul, 0) + M(" ödül", " reward"), RENK_ALTIN, 5.0f);
    o.parcaciklar.Patlat({(float)PENCERE_GENISLIK / 2.0f, 220.0f}, RENK_ALTIN, 30, 240.0f);
    Cal(o.sesler.basarim);
    o.sonGorevTur = o.gorevTur;
    o.gorevTur = G_YOK;
    o.gorevBekleme = GOREV_ARA_SN;
    if (o.tamamlananGorev >= 10) BasarimVer(o, B_GOREV);
    RutbeVeHedefKontrolEt(o);
}

// Oyunda bir sey oldu; aktif gorevin kosuluysa tamamlar. `deger` tutar gerektiren
// gorevler icindir (kar, temettu).
static void GorevOlay(Oyun& o, int tur, float deger = 0.0f) {
    if (o.gorevTur != tur || tur == G_YOK) return;
    if ((tur == G_KAR_ET || tur == G_TEMETTU) && deger < o.gorevHedef) return;
    GorevTamamla(o);
}

// Her simulasyon adiminda: yeni gorev verme, yoklanan kosullar, sure dolmasi.
static void GorevGuncelle(Oyun& o, float dt) {
    if (o.rehberAdim >= 0) return; // rehber bitene kadar gorev yok (oyuncuyu bogmayalim)
    if (o.gorevTur == G_YOK) {
        o.gorevBekleme -= dt;
        if (o.gorevBekleme <= 0.0f) GorevVer(o);
        return;
    }
    if (o.gorevTur == G_CESITLENDIR && o.PozisyonSayisi() >= (int)o.gorevHedef) { GorevTamamla(o); return; }
    if (o.gorevTur == G_NET_DEGER && o.NetDeger() >= o.gorevHedef) { GorevTamamla(o); return; }
    if (o.gun > o.gorevBitisGunu) {
        // Sure doldu: ceza yok, yenisi gelir.
        o.sonGorevTur = o.gorevTur;
        o.gorevTur = G_YOK;
        o.gorevBekleme = GOREV_ARA_SN;
    }
}
