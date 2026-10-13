// ============================================================================
// piyasa_olay.h - fiyat simulasyonu, haberler, degerleme, kuresel olaylar,
//                 bilanco, iceriden bilgi ve rutbe ilerlemesi
// ============================================================================
// RASTGELELIK KURALI: bu dosyadaki ortam olaylari g_rngPiyasa'dan, oyuncunun
// durumundan BAGIMSIZ sayida cekim yapar (kilitli hisse icin bile zar atilir,
// sonucu kullanilmaz). Boylece ayni tohumla baslayan iki oyuncu - rutbeleri
// farkli olsa da - ayni kuresel olaylari ve ayni piyasa modunu gorur.

static void RutbeVeHedefKontrolEt(Oyun& o) {
    float net = o.NetDeger();
    if (net < o.enDusukNetDeger) o.enDusukNetDeger = net;

    int yeniRutbe = o.rutbeIndex;
    for (size_t i = 0; i < RUTBELER.size(); i++)
        if (net >= RUTBELER[i].esikNetDeger) yeniRutbe = (int)i;
    if (yeniRutbe <= o.rutbeIndex) return;

    int eskiRutbe = o.rutbeIndex;
    o.rutbeIndex = yeniRutbe;
    o.hedefIndex = yeniRutbe; // (eski kayit formatiyla uyum icin tutuluyor)
    const bool sonRutbe = (yeniRutbe == (int)RUTBELER.size() - 1);

    string govde = RUTBELER[yeniRutbe].ad.al();
    if (!sonRutbe)
        govde += string(M("  -  yeni hedef ", "  -  next goal ")) + Para(RUTBELER[yeniRutbe + 1].esikNetDeger, 0);
    o.toastlar.Ekle(M("RÜTBE ATLADIN!", "RANK UP!"), govde, RENK_ALTIN, 5.0f);
    HaberBandinaEkle(o, string(M("TEBRİKLER: Yeni rütben '", "CONGRATULATIONS: Your new rank is '")) +
                             RUTBELER[yeniRutbe].ad.al() + "'!");
    ParlamaBaslat(o, RENK_ALTIN);
    Cal(o.sesler.rutbeAtlama);

    if (o.enDusukNetDeger <= 0.6f * o.BaslangicSermayesi()) BasarimVer(o, B_DONUS);

    // Bu rutbe atlayisinda acilan yeni hisse(ler) (birden fazla rutbe birden
    // atlanirsa aradaki hepsi tek seferde acilir).
    for (int i = 0; i < HisseSayisi(); i++) {
        Hisse& h = o.hisseler[i];
        if (h.acilmaSeviyesi > eskiRutbe && h.acilmaSeviyesi <= yeniRutbe) {
            o.toastlar.Ekle(M("YENİ HİSSE AÇILDI!", "NEW STOCK UNLOCKED!"), h.kod + " - " + h.tamAd.al(), h.renk, 5.0f);
            HaberBandinaEkle(o, string(M("YENİ HİSSE: ", "NEW STOCK: ")) + h.kod + " (" + h.tamAd.al() +
                                     M(") artık işlem görüyor!", ") is now trading!"));
        }
    }

    // Bu rutbe atlayisinda acilan yeni ARACLAR.
    for (int r = eskiRutbe + 1; r <= yeniRutbe; r++) {
        if (r == EMIR_MIN_RUTBE) {
            o.toastlar.Ekle(M("YENİ ARAÇ: EMİRLER", "NEW TOOL: ORDERS"),
                            M("EMİR düğmesiyle zarar kes, kâr al ve limit emri ver.",
                              "Use the ORDER button for stop-loss, target and limit orders."),
                            RENK_MAVI_VURGU, 8.0f);
            HaberBandinaEkle(o, M("YENİ ARAÇ: Bekleyen emirler açıldı - sen ekrana bakmazken de pozisyonun korunur.",
                                  "NEW TOOL: Pending orders unlocked - your position stays protected while you look away."));
        }
        if (r == ACIGA_MIN_RUTBE) {
            o.toastlar.Ekle(M("YENİ ARAÇ: AÇIĞA SATIŞ", "NEW TOOL: SHORT SELLING"),
                            M("Elinde olmayan hisseyi SAT ile açığa sat; düşüşten kazan.",
                              "Press SELL on a stock you do not hold to short it and profit from a fall."),
                            RENK_MAVI_VURGU, 8.0f);
            HaberBandinaEkle(o, M("YENİ ARAÇ: Açığa satış açıldı - kötü haberden de kazanabilirsin (zarar sınırsızdır, dikkat).",
                                  "NEW TOOL: Short selling unlocked - profit from bad news too (losses are unlimited, careful)."));
        }
        if (r == LIMAN_MIN_RUTBE) {
            o.toastlar.Ekle(M("YENİ VARLIK: ALTIN ve DOLAR", "NEW ASSETS: GOLD and DOLLAR"),
                            M("Piyasa düşerken yükselirler; kötü günde sığınak.",
                              "They rise when the market falls; shelter on a bad day."),
                            RENK_MAVI_VURGU, 8.0f);
            HaberBandinaEkle(o, M("GÜVENLİ LİMAN: Altın ve dolar açıldı - kötü küresel haberlerde hisselerin tersine hareket ederler.",
                                  "SAFE HAVENS: Gold and the dollar are unlocked - on bad global news they move against stocks."));
        }
        if (r == KREDI_MIN_RUTBE) {
            o.toastlar.Ekle(M("YENİ ARAÇ: KREDİ", "NEW TOOL: CREDIT"),
                            M("Kredi düğmesiyle borç al; kazanç da zarar da büyür.",
                              "Borrow with the Credit button; gains and losses both grow."),
                            RENK_MAVI_VURGU, 8.0f);
            HaberBandinaEkle(o, M("YENİ ARAÇ: Kredi masası açıldı - teminat oranın düşerse pozisyonların zorla kapatılır.",
                                  "NEW TOOL: The credit desk is open - if your margin ratio drops, positions are force-closed."));
        }
    }

    // ---- ZIRVE: son rutbe ----
    if (sonRutbe && !o.zirveyeUlasildi) {
        o.zirveyeUlasildi = true;
        HaberBandinaEkle(o, string(M("BORSA TARİHİNE GEÇTİN: ", "YOU MADE MARKET HISTORY: ")) +
                                 Para(RUTBELER.back().esikNetDeger, 0) +
                                 M(" net değerle zirveye ulaştın!", " in net worth - you reached the top!"));
        o.parcaciklar.Patlat({(float)PENCERE_GENISLIK / 2.0f, 260.0f}, RENK_ALTIN, 80, 320.0f);
        BasarimVer(o, B_EFSANE);
        if (o.senaryo == S_KLASIK && o.gun < HIZ_REKORU_GUN) BasarimVer(o, B_HIZLI);
        if (o.senaryo == S_KRIZ) BasarimVer(o, B_KRIZ);
        // Sureli senaryoda oyun zirvede degil sure dolunca biter.
        if (o.Sen().gunSiniri == 0) {
            o.sonTuru = SON_ZIRVE;
            o.sonPenceresiAcik = true;
            o.bitisGunu = o.gun;
            o.bitisNetDeger = net;
            o.yeniRekor = RekorGunGuncelle(o.senaryo, o.gun);
        }
    }
}

// ---------------------------------------------------------------------------
// FIYAT SIMULASYONU (saniyede bir)
// ---------------------------------------------------------------------------
static void PiyasaGuncelle(Oyun& o) {
    // 1) Ortak piyasa faktoru: yavas degisen, kalici (AR(1)).
    o.piyasaFaktoru = o.piyasaFaktoru * MARKET_KALICILIK + RastgeleGauss() * MARKET_SOK_BUYUKLUK;

    for (auto& h : o.hisseler) {
        h.oncekiFiyat = h.fiyat;

        // 2) Varliga ozgu KALICI faktor (kendi trendi) ve beyaz gurultu.
        h.ozelFaktor = h.ozelFaktor * OZEL_KALICILIK + RastgeleGauss() * OZEL_SOK_BUYUKLUK * h.ozelOlcek;
        float beyazGurultu = RastgeleGauss() * h.volatilite * h.volCarpani;

        // 3) Genel piyasadan betasi kadar pay (guvenli limanlarda beta EKSI:
        // piyasa duserken hafifce yukselirler).
        float piyasaEtkisi = o.piyasaFaktoru * h.beta;

        // 4) Adil degere dogru hafif cekim.
        float sapmaOrani = (h.fiyat - h.temelDeger) / h.temelDeger;
        float cekimEtkisi = -sapmaOrani * REVERSION_GUCU;

        float toplamOran = beyazGurultu + h.ozelFaktor + piyasaEtkisi + cekimEtkisi;
        h.fiyat = fmaxf(0.5f, h.fiyat * (1.0f + toplamOran));
        h.gecmis.push_back(h.fiyat);
        if ((int)h.gecmis.size() > GECMIS_UZUNLUK) h.gecmis.pop_front();
        h.gosterilenFiyat.Ayarla(h.fiyat);

        // 5) Oynaklik kumelenmesi: buyuk hareket sonrasi oynaklik sicrar, sakin
        // turlerde tabanina soner. Taban = baz + varliga ozel risk primi + kuresel
        // risk primi (ikisi de KALICI; oyun ilerledikce piyasa sertlesir).
        if (fabsf(toplamOran) > VOL_SOK_ESIGI) {
            h.volatilite = fminf(VOL_TAVAN, h.volatilite * VOL_SOK_CARPANI);
        } else {
            float taban = fminf(VOL_TABAN + h.riskPrimi + o.kureselRiskPrimi, VOL_TAVAN);
            h.volatilite = fmaxf(taban, h.volatilite * VOL_SONME_HIZI);
        }

        // 6) Oyuncunun emir baskisi soner (emir defteri yeniden dolar).
        h.akis *= AKIS_SONME;
        if (fabsf(h.akis) < 0.5f) h.akis = 0.0f;
    }
    RutbeVeHedefKontrolEt(o);
}

// Bir olay havuzundan, IYI (carpan > 1) olaylara 'iyiSans' olasilikla agirlik
// vererek bir eleman secer. Istenen turden olay yoksa digerine duser.
// Her zaman tam IKI rastgele sayi ceker.
template <typename T, typename CarpanAl>
static int AgirlikliOlaySec(const vector<T>& havuz, float iyiSans, CarpanAl carpanAl) {
    float zarYon = Rastgele01();
    float zarSira = Rastgele01();
    if (havuz.empty()) return 0;
    vector<int> iyiler, kotular;
    for (int i = 0; i < (int)havuz.size(); i++) {
        if (carpanAl(havuz[i]) > 1.0f) iyiler.push_back(i);
        else kotular.push_back(i);
    }
    bool iyiIstiyoruz = zarYon < iyiSans;
    const vector<int>* grup = nullptr;
    if (iyiIstiyoruz && !iyiler.empty()) grup = &iyiler;
    else if (!iyiIstiyoruz && !kotular.empty()) grup = &kotular;
    else grup = iyiler.empty() ? &kotular : &iyiler;
    int n = (int)grup->size();
    return (*grup)[min(n - 1, (int)(zarSira * (float)n))];
}

// Acik (kilidi acilmis) HISSELER. Guvenli limanlar dahil degildir: onlarin
// haberi, bilancosu, iceriden bilgisi yoktur.
static vector<int> AcikHisseler(const Oyun& o) {
    vector<int> acik;
    for (int i = 0; i < HisseSayisi(); i++)
        if (o.HisseAcikMi(i)) acik.push_back(i);
    return acik;
}

// Haber bandini bu habere SABITLE: haber ekranin sol kenarindan tam gorunecek
// sekilde konumlanir ve `sure` saniye boyunca kaymaz (oyuncu okuyabilsin).
static void BandiSabitle(Oyun& o, float sure) {
    o.bantKaydirma = (float)PENCERE_GENISLIK;
    o.haberSabitKalmaSuresi = sure;
}

// Haberlerin HEYECAN payinin rutbeye gore olcegi: rutbe 0'da 0, son rutbede 1.
// (Senaryonun acemi korumasindan BAGIMSIZ: Kriz Yili'nda da rutbeyle buyur.)
// Yalnizca degerlemenin ne kadar hizli OLUSTUGUNU belirler; olusmus bir
// degerlemenin cezasi her rutbede aynidir.
static float DegerlemeGucu(const Oyun& o) {
    int sonRutbe = (int)RUTBELER.size() - 1;
    return (sonRutbe > 0) ? Clamp((float)o.rutbeIndex / (float)sonRutbe, 0.0f, 1.0f) : 0.0f;
}

static void HaberKontrolEt(Oyun& o) {
    float zarVar = Rastgele01();
    float zarHisse = Rastgele01();
    if (zarVar >= HABER_OLASILIK) return;
    // Haberler sadece ACIK hisseler icin cikar (kilitli hisse spoiler olmasin).
    vector<int> acik = AcikHisseler(o);
    if (acik.empty()) {
        Rastgele01(); // akis kaymasin: AgirlikliOlaySec'in iki cekimi
        Rastgele01();
        return;
    }
    int idx = acik[min((int)acik.size() - 1, (int)(zarHisse * (float)acik.size()))];
    const auto& havuz = SIRKETLER[idx].haberler;

    // Iyi haber olasiligi: rutbeye bagli taban (acemi korumasi) EKSI degerleme
    // cezasi. Pahali hisseye iyi haber gelmesi zorlasir, ucuz hisseye kolaylasir.
    float iyiSans = ACEMI_HISSE_IYI_TABAN + ACEMI_HISSE_IYI_BONUS * AcemiKorumasi(o);
    iyiSans -= DEGERLEME_HABER_EGIMI * logf(Degerleme(o.hisseler[idx]));
    iyiSans = Clamp(iyiSans, 0.10f, 0.92f);
    const auto& sablon = havuz[AgirlikliOlaySec(havuz, iyiSans, [](const HaberSablonu& s) { return s.carpan; })];

    BeklemedeHaber bh;
    bh.hisseIndex = idx;
    bh.carpan = sablon.carpan;
    bh.metin = sablon.metin.al();
    bh.kalanSure = HABER_ETKI_GECIKME_SN;
    bh.tur = HABER_SIRKET;
    o.bekleyenHaberler.push_back(bh);

    HaberBandinaEkle(o, string(M("SON DAKİKA [", "BREAKING [")) + o.hisseler[idx].kod + "]: " + bh.metin + "  (" +
                             to_string((int)HABER_ETKI_GECIKME_SN) +
                             M(" sn içinde fiyata yansıyacak)", " s until it hits the price)"));
    Cal(o.sesler.haberDuyuru);
    BandiSabitle(o, HABER_ETKI_GECIKME_SN);
}

static void BekleyenHaberleriGuncelle(Oyun& o, float dt) {
    for (size_t k = 0; k < o.bekleyenHaberler.size();) {
        BeklemedeHaber& bh = o.bekleyenHaberler[k];
        bh.kalanSure -= dt;
        if (bh.kalanSure > 0.0f) {
            k++;
            continue;
        }
        const float carpan = bh.carpan;
        const int tur = bh.tur;
        Hisse& h = o.hisseler[bh.hisseIndex];
        h.fiyat = fmaxf(0.5f, h.fiyat * carpan);
        // Haber adil degeri de kaydirir - yoksa fiyat birkac saniyede geri surunur.
        h.temelDeger = fmaxf(0.5f, h.temelDeger * carpan);

        // ESAS DEGER (degerleme): haberin ne kadari "gercek"?
        //   bilanco   : tamami (sirketin gercek kazanci)
        //   duzeltme  : hic (fiyat esasa yaklasir, esas degismez)
        //   haber vb. : rutbeyle artan bir kismi HEYECAN'dir, esasa yansimaz
        if (tur == HABER_BILANCO) {
            h.esasDeger = fmaxf(0.5f, h.esasDeger * carpan);
        } else if (tur != HABER_DUZELTME) {
            float heyecan = DEGERLEME_HEYECAN_PAYI * DegerlemeGucu(o);
            h.esasDeger = fmaxf(0.5f, h.esasDeger * powf(carpan, 1.0f - heyecan));
        }

        h.volatilite = fminf(VOL_TAVAN, h.volatilite * VOL_SOK_CARPANI);
        h.gosterilenFiyat.Ayarla(h.fiyat);
        h.flashSuresi = 0.6f;
        h.flashIyi = carpan > 1.0f;
        Cal(o.sesler.haberEtki);
        const bool elde = h.adet > 0;
        const bool haberdenSonraAlindi = bh.oyuncuAldi;
        o.bekleyenHaberler.erase(o.bekleyenHaberler.begin() + k); // (bh ve h referanslari artik kullanilmiyor)

        if (carpan > 1.0f && elde) {
            if (tur == HABER_BILANCO) GorevOlay(o, G_BILANCO);
            else if (tur == HABER_SIRKET && haberdenSonraAlindi) GorevOlay(o, G_HABER);
        }
    }
}

// ---------------------------------------------------------------------------
// DEGERLEME: duzeltme ve toparlanma (yeni gunde)
// ---------------------------------------------------------------------------
static void DegerlemeKontrolEt(Oyun& o) {
    for (int i = 0; i < HisseSayisi(); i++) {
        float zar = Rastgele01(); // kilitli hisse icin de cekilir (akis sabit kalsin)
        Hisse& h = o.hisseler[i];
        if (!o.HisseAcikMi(i)) continue;

        bool bekleyenVar = false; // ayni hissede ust uste duzeltme birikmesin
        for (auto& bh : o.bekleyenHaberler)
            if (bh.hisseIndex == i && bh.tur == HABER_DUZELTME) bekleyenVar = true;
        if (bekleyenVar) continue;

        const float v = Degerleme(h);
        float carpan = 1.0f;
        const char* metin = nullptr;
        const char* baslik = nullptr;
        if (v > DUZELTME_ESIGI && zar < fminf(0.5f, DUZELTME_OLASILIK_EGIMI * (v - DUZELTME_ESIGI))) {
            carpan = Clamp(powf(v, -0.5f), 0.80f, 0.94f); // fazlaligin yarisi geri alinir
            baslik = M("DÜZELTME [", "CORRECTION [");
            metin = M("Kâr satışları başladı; değerleme fazla şişmişti.", "Profit-taking has begun; the valuation had run too hot.");
        } else if (v < TOPARLANMA_ESIGI && zar < fminf(0.5f, DUZELTME_OLASILIK_EGIMI * (TOPARLANMA_ESIGI - v))) {
            carpan = Clamp(powf(v, -0.5f), 1.06f, 1.22f);
            baslik = M("TOPARLANMA [", "REBOUND [");
            metin = M("Ucuz kalan hisseye alıcı geldi.", "Buyers are stepping into the beaten-down stock.");
        }
        if (metin == nullptr) continue;

        BeklemedeHaber bh;
        bh.hisseIndex = i;
        bh.carpan = carpan;
        bh.metin = metin;
        bh.kalanSure = HABER_ETKI_GECIKME_SN;
        bh.tur = HABER_DUZELTME;
        o.bekleyenHaberler.push_back(bh);
        HaberBandinaEkle(o, string(baslik) + h.kod + "]: " + metin + "  (" + to_string((int)HABER_ETKI_GECIKME_SN) +
                                 M(" sn içinde fiyata yansıyacak)", " s until it hits the price)"));
        Cal(o.sesler.haberDuyuru);
        BandiSabitle(o, HABER_ETKI_GECIKME_SN);
    }
}

// ---------------------------------------------------------------------------
// BILANCO TAKVIMI
// ---------------------------------------------------------------------------
// Yeni bir gune gecildiginde cagrilir.
static void BilancoKontrolEt(Oyun& o) {
    for (int i = 0; i < HisseSayisi(); i++) {
        Hisse& h = o.hisseler[i];
        if (o.gun < h.bilancoGunu) continue;
        const int beklenti = h.bilancoBeklenti;
        // Zarlar kilitli hisse icin de atilir (ortam akisi oyuncudan bagimsiz kalsin).
        const float zarSonuc = Rastgele01();
        const float zarBuyukluk = Rastgele01();
        h.bilancoGunu = o.gun + BILANCO_PERIYOT_GUN;
        h.bilancoBeklenti = BilancoBeklentiUret(AcemiKorumasi(o));
        if (!o.HisseAcikMi(i)) continue; // kilitli hissenin bilancosu sessizce gecer

        bool iyi;
        float buyukluk;
        if (beklenti == 0) {
            iyi = zarSonuc < 0.5f;
            buyukluk = BILANCO_BELIRSIZ_MIN + (BILANCO_BELIRSIZ_MAX - BILANCO_BELIRSIZ_MIN) * zarBuyukluk;
        } else {
            bool tuttu = zarSonuc < BILANCO_TUTMA_OLASILIGI;
            iyi = (beklenti > 0) == tuttu;
            buyukluk = tuttu ? BILANCO_UYUMLU_MIN + (BILANCO_UYUMLU_MAX - BILANCO_UYUMLU_MIN) * zarBuyukluk
                             : BILANCO_SURPRIZ_MIN + (BILANCO_SURPRIZ_MAX - BILANCO_SURPRIZ_MIN) * zarBuyukluk;
        }

        BeklemedeHaber bh;
        bh.hisseIndex = i;
        bh.carpan = iyi ? (1.0f + buyukluk) : (1.0f - buyukluk);
        bh.metin = iyi ? M("Bilanço beklentileri aştı, kâr güçlü geldi.", "Earnings beat expectations; profit came in strong.")
                       : M("Bilanço hayal kırıklığı yarattı, kâr beklentinin altında.", "Earnings disappointed; profit missed expectations.");
        bh.kalanSure = HABER_ETKI_GECIKME_SN;
        bh.tur = HABER_BILANCO;
        o.bekleyenHaberler.push_back(bh);

        HaberBandinaEkle(o, string(M("BİLANÇO [", "EARNINGS [")) + h.kod + "]: " + bh.metin + "  (" +
                                 to_string((int)HABER_ETKI_GECIKME_SN) +
                                 M(" sn içinde fiyata yansıyacak)", " s until it hits the price)"));
        o.toastlar.Ekle(M("BİLANÇO AÇIKLANDI", "EARNINGS RELEASED"), h.kod + ": " + bh.metin, RENK_UYARI, 5.0f);
        Cal(o.sesler.haberDuyuru);
        BandiSabitle(o, HABER_ETKI_GECIKME_SN);
    }
}

// ---------------------------------------------------------------------------
// ICERIDEN BILGI MEKTUBU (oyuncuya ozel -> g_rngOyuncu)
// ---------------------------------------------------------------------------
static void InsiderMektupKontrolEt(Oyun& o) {
    if (g_rngOyuncu.Birim() >= INSIDER_OLASILIK) return;
    vector<int> acik = AcikHisseler(o);
    if (acik.empty()) return;

    int idx = acik[g_rngOyuncu.Aralik(0, (int)acik.size() - 1)];
    float artis = INSIDER_ARTIS_MIN + (INSIDER_ARTIS_MAX - INSIDER_ARTIS_MIN) * g_rngOyuncu.Birim();
    float gecikme = INSIDER_GECIKME_MIN_SN + (INSIDER_GECIKME_MAX_SN - INSIDER_GECIKME_MIN_SN) * g_rngOyuncu.Birim();

    // Yukselisi SIMDIDEN kuyruga koy: mektup okunmasa da gerceklesecek.
    BeklemedeHaber bh;
    bh.hisseIndex = idx;
    bh.carpan = 1.0f + artis;
    bh.metin = "(insider)";
    bh.kalanSure = gecikme;
    bh.tur = HABER_INSIDER;
    o.bekleyenHaberler.push_back(bh);

    o.insiderMektupVar = true;
    o.insiderMektupOmur = INSIDER_MEKTUP_OMUR_SN;
    o.insiderOkundu = false;
    o.insiderHisseIndex = idx;
    o.insiderArtisOrani = artis;
    o.insiderKalanSure = gecikme;

    // Haber bandina BILEREK yazilmaz - bu bilgi sadece oyuncuya geliyor.
    o.toastlar.Ekle(M("GİZLİ ZARF", "SECRET ENVELOPE"),
                    M("Seni bekleyen bir mektup var.", "A letter is waiting for you."), RENK_ALTIN, 5.0f);
    Cal(o.sesler.haberDuyuru);
}

// Oyuncu zarfa tikladi: bilgiyi acar (yukselisi DEGISTIRMEZ).
static void InsiderMektubuAc(Oyun& o) {
    if (!o.insiderMektupVar || o.insiderHisseIndex < 0) return;
    o.insiderMektupVar = false;
    o.insiderOkundu = true;
    o.toastlar.Ekle(M("İÇERİDEN BİLGİ", "INSIDER TIP"),
                    o.hisseler[o.insiderHisseIndex].kod + " " + Yuzde(o.insiderArtisOrani * 100.0f, 1, true) + "  (~" +
                        to_string((int)(o.insiderKalanSure + 0.5f)) + M(" sn)", " s)"),
                    RENK_ALTIN, 7.0f);
    Cal(o.sesler.rutbeAtlama);
    BasarimVer(o, B_ZARF);
}

// ---------------------------------------------------------------------------
// KURESEL (MAKRO) OLAYLAR
// ---------------------------------------------------------------------------
static void KureselHaberKontrolEt(Oyun& o) {
    float zarVar = Rastgele01();
    // Gunun Piyasasi'nda kuresel olaylar HERKES icin ayni olmali; orada iyi/kotu
    // dengesi oyuncunun rutbesine degil sabit bir orta degere baglidir.
    float koruma = o.Sen().gunluk ? 0.5f : AcemiKorumasi(o);
    float iyiSans = ACEMI_KURESEL_IYI_TABAN + ACEMI_KURESEL_IYI_BONUS * koruma;
    // (Olay olmasa da secim zarlari atilir: akis oyuncudan bagimsiz kalsin.)
    int secilen = AgirlikliOlaySec(KURESEL_OLAYLARI, iyiSans, [](const KureselOlaySablonu& k) { return k.temelCarpan; });
    if (zarVar >= KURESEL_OLAY_OLASILIK) return;
    const auto& sablon = KURESEL_OLAYLARI[secilen];

    BeklemedeKuresel bk;
    bk.carpan = sablon.temelCarpan;
    bk.riskPrimiDegisimi = sablon.riskPrimiDegisimi;
    bk.metin = sablon.metin.al();
    bk.kalanSure = KURESEL_OLAY_ETKI_GECIKME_SN;
    bk.vurgulananSektor = sablon.vurgulananSektor;
    bk.sektorCarpani = sablon.sektorCarpani;
    o.bekleyenKureselHaberler.push_back(bk);

    // BILEREK yesil/kirmizi KULLANILMIYOR: haberin iyi mi kotu mu oldugunu
    // oyuncu OKUYUP tahmin etmeli; sonucu ancak fiyatlara yansiyinca ogrenir.
    HaberBandinaEkle(o, string(M("KÜRESEL GELİŞME: ", "GLOBAL NEWS: ")) + bk.metin + "  (" +
                             to_string((int)KURESEL_OLAY_ETKI_GECIKME_SN) +
                             M(" sn içinde piyasaya yansıyacak)", " s until it hits the market)"));
    o.toastlar.Ekle(M("KÜRESEL GELİŞME!", "GLOBAL NEWS!"), bk.metin, RENK_UYARI, 6.0f);
    Cal(o.sesler.haberDuyuru);
    ParlamaBaslat(o, RENK_UYARI);
    BandiSabitle(o, KURESEL_OLAY_ETKI_GECIKME_SN);
}

static void BekleyenKureselHaberleriGuncelle(Oyun& o, float dt) {
    for (size_t k = 0; k < o.bekleyenKureselHaberler.size();) {
        BeklemedeKuresel& bk = o.bekleyenKureselHaberler[k];
        bk.kalanSure -= dt;
        if (bk.kalanSure > 0.0f) {
            k++;
            continue;
        }
        bool limanTutuluyor = false;
        for (auto& h : o.hisseler) {
            // Etki her varlikta kendi kuresel duyarliligiyla olceklenir: hisselerde
            // beta (ayni yon), guvenli limanlarda EKSI (ters yon). Vurgulanan sektor
            // ayrica (bazen ters yonde) etkilenir.
            float etki = (bk.carpan - 1.0f) * h.kureselDuyarlilik;
            if (!h.liman && bk.vurgulananSektor != Sektor::YOK && h.sektor == bk.vurgulananSektor)
                etki += (bk.sektorCarpani - 1.0f);
            h.fiyat = fmaxf(0.5f, h.fiyat * (1.0f + etki));
            // Hisselerde etki kalicidir (makro, sirketin gercek degerini degistirir).
            // Limanlarda sadece bir kismi kalici: panik gecince fiyat kismen geri gelir.
            float kalici = 1.0f + etki * h.kaliciPay;
            h.temelDeger = fmaxf(0.5f, h.temelDeger * kalici);
            h.esasDeger = fmaxf(0.5f, h.esasDeger * kalici);
            h.volatilite = fminf(VOL_TAVAN, h.volatilite * VOL_SOK_CARPANI);
            h.gosterilenFiyat.Ayarla(h.fiyat);
            h.flashSuresi = 0.6f;
            h.flashIyi = etki >= 0.0f;
            if (h.liman && h.adet > 0) limanTutuluyor = true;
        }
        o.kureselRiskPrimi = Clamp(o.kureselRiskPrimi + bk.riskPrimiDegisimi, KURESEL_RISK_PRIMI_TABANI, KURESEL_RISK_PRIMI_TAVANI);
        const bool kotu = bk.carpan < 1.0f;
        Cal(o.sesler.haberEtki);
        o.bekleyenKureselHaberler.erase(o.bekleyenKureselHaberler.begin() + k);

        if (kotu && limanTutuluyor) {
            BasarimVer(o, B_LIMAN);
            GorevOlay(o, G_LIMAN);
        }
    }
}
