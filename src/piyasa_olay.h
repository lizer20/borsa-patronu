// ============================================================================
// piyasa_olay.h - fiyat simulasyonu, haberler, kuresel olaylar, bilanco,
//                 iceriden bilgi, rutbe ilerlemesi ve SimulasyonAdimi()
// ============================================================================

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
    for (auto& h : o.hisseler) {
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
                            M("Kartlardaki EMİR düğmesiyle zarar kes, kâr al ve limit emri ver.",
                              "Use the ORDER button on cards for stop-loss, target and limit orders."),
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
        if (r == KREDI_MIN_RUTBE) {
            o.toastlar.Ekle(M("YENİ ARAÇ: KREDİ", "NEW TOOL: CREDIT"),
                            M("Sağ üstteki Kredi düğmesiyle borç al; kazanç da zarar da büyür.",
                              "Borrow with the Credit button at the top right; gains and losses both grow."),
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

        // 2) Hisseye ozgu KALICI faktor (kendi trendi) ve beyaz gurultu.
        float ozelOlcek = OZEL_OLCEK_TABAN + OZEL_OLCEK_EGIM * h.beta;
        h.ozelFaktor = h.ozelFaktor * OZEL_KALICILIK + RastgeleGauss() * OZEL_SOK_BUYUKLUK * ozelOlcek;
        float beyazGurultu = RastgeleGauss() * h.volatilite;

        // 3) Genel piyasadan betasi kadar pay.
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
        // turlerde tabanina soner. Taban = baz + hisseye ozel risk primi + kuresel
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
template <typename T, typename CarpanAl>
static int AgirlikliOlaySec(const vector<T>& havuz, float iyiSans, CarpanAl carpanAl) {
    if (havuz.empty()) return 0;
    vector<int> iyiler, kotular;
    for (int i = 0; i < (int)havuz.size(); i++) {
        if (carpanAl(havuz[i]) > 1.0f) iyiler.push_back(i);
        else kotular.push_back(i);
    }
    bool iyiIstiyoruz = Rastgele01() < iyiSans;
    const vector<int>* grup = nullptr;
    if (iyiIstiyoruz && !iyiler.empty()) grup = &iyiler;
    else if (!iyiIstiyoruz && !kotular.empty()) grup = &kotular;
    else grup = iyiler.empty() ? &kotular : &iyiler;
    if (grup->empty()) return GetRandomValue(0, (int)havuz.size() - 1);
    return (*grup)[GetRandomValue(0, (int)grup->size() - 1)];
}

static vector<int> AcikHisseler(const Oyun& o) {
    vector<int> acik;
    for (int i = 0; i < (int)o.hisseler.size(); i++)
        if (o.HisseAcikMi(i)) acik.push_back(i);
    return acik;
}

// Haber bandini bu habere SABITLE: haber ekranin sol kenarindan tam gorunecek
// sekilde konumlanir ve `sure` saniye boyunca kaymaz (oyuncu okuyabilsin).
static void BandiSabitle(Oyun& o, float sure) {
    o.bantKaydirma = (float)PENCERE_GENISLIK;
    o.haberSabitKalmaSuresi = sure;
}

static void HaberKontrolEt(Oyun& o) {
    if (Rastgele01() >= HABER_OLASILIK) return;
    // Haberler sadece ACIK hisseler icin cikar (kilitli hisse spoiler olmasin).
    vector<int> acik = AcikHisseler(o);
    if (acik.empty()) return;
    int idx = acik[GetRandomValue(0, (int)acik.size() - 1)];
    const auto& havuz = SIRKETLER[idx].haberler;
    float iyiSans = ACEMI_HISSE_IYI_TABAN + ACEMI_HISSE_IYI_BONUS * AcemiKorumasi(o);
    const auto& sablon = havuz[AgirlikliOlaySec(havuz, iyiSans, [](const HaberSablonu& s) { return s.carpan; })];

    BeklemedeHaber bh;
    bh.hisseIndex = idx;
    bh.carpan = sablon.carpan;
    bh.metin = sablon.metin.al();
    bh.kalanSure = HABER_ETKI_GECIKME_SN;
    o.bekleyenHaberler.push_back(bh);

    HaberBandinaEkle(o, string(M("SON DAKİKA [", "BREAKING [")) + o.hisseler[idx].kod + "]: " + bh.metin + "  (" +
                             to_string((int)HABER_ETKI_GECIKME_SN) +
                             M(" sn içinde fiyata yansıyacak)", " s until it hits the price)"));
    Cal(o.sesler.haberDuyuru);
    BandiSabitle(o, HABER_ETKI_GECIKME_SN);
}

static void BekleyenHaberleriGuncelle(Oyun& o, float dt) {
    for (auto it = o.bekleyenHaberler.begin(); it != o.bekleyenHaberler.end();) {
        it->kalanSure -= dt;
        if (it->kalanSure <= 0.0f) {
            Hisse& h = o.hisseler[it->hisseIndex];
            h.fiyat = fmaxf(0.5f, h.fiyat * it->carpan);
            // Haber adil degeri de kaydirir - yoksa fiyat birkac saniyede geri surunur.
            h.temelDeger = fmaxf(0.5f, h.temelDeger * it->carpan);
            h.volatilite = fminf(VOL_TAVAN, h.volatilite * VOL_SOK_CARPANI);
            h.gosterilenFiyat.Ayarla(h.fiyat);
            h.flashSuresi = 0.6f;
            h.flashIyi = it->carpan > 1.0f;
            Cal(o.sesler.haberEtki);
            it = o.bekleyenHaberler.erase(it);
        } else {
            ++it;
        }
    }
}

// ---------------------------------------------------------------------------
// BILANCO TAKVIMI
// ---------------------------------------------------------------------------
static void BilancoYenidenPlanla(Oyun& o, Hisse& h) {
    h.bilancoGunu = o.gun + BILANCO_PERIYOT_GUN;
    h.bilancoBeklenti = BilancoBeklentiUret(AcemiKorumasi(o));
}

// Yeni bir gune gecildiginde cagrilir.
static void BilancoKontrolEt(Oyun& o) {
    for (int i = 0; i < (int)o.hisseler.size(); i++) {
        Hisse& h = o.hisseler[i];
        if (o.gun < h.bilancoGunu) continue;
        int beklenti = h.bilancoBeklenti;
        BilancoYenidenPlanla(o, h);
        if (!o.HisseAcikMi(i)) continue; // kilitli hissenin bilancosu sessizce gecer

        bool iyi;
        float buyukluk;
        if (beklenti == 0) {
            iyi = Rastgele01() < 0.5f;
            buyukluk = RastgeleAralik(BILANCO_BELIRSIZ_MIN, BILANCO_BELIRSIZ_MAX);
        } else {
            bool tuttu = Rastgele01() < BILANCO_TUTMA_OLASILIGI;
            iyi = (beklenti > 0) == tuttu;
            buyukluk = tuttu ? RastgeleAralik(BILANCO_UYUMLU_MIN, BILANCO_UYUMLU_MAX)
                             : RastgeleAralik(BILANCO_SURPRIZ_MIN, BILANCO_SURPRIZ_MAX);
        }

        BeklemedeHaber bh;
        bh.hisseIndex = i;
        bh.carpan = iyi ? (1.0f + buyukluk) : (1.0f - buyukluk);
        bh.metin = iyi ? M("Bilanço beklentileri aştı, kâr güçlü geldi.", "Earnings beat expectations; profit came in strong.")
                       : M("Bilanço hayal kırıklığı yarattı, kâr beklentinin altında.", "Earnings disappointed; profit missed expectations.");
        bh.kalanSure = HABER_ETKI_GECIKME_SN;
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
// ICERIDEN BILGI MEKTUBU
// ---------------------------------------------------------------------------
static void InsiderMektupKontrolEt(Oyun& o) {
    if (Rastgele01() >= INSIDER_OLASILIK) return;
    vector<int> acik = AcikHisseler(o);
    if (acik.empty()) return;

    int idx = acik[GetRandomValue(0, (int)acik.size() - 1)];
    float artis = RastgeleAralik(INSIDER_ARTIS_MIN, INSIDER_ARTIS_MAX);
    float gecikme = RastgeleAralik(INSIDER_GECIKME_MIN_SN, INSIDER_GECIKME_MAX_SN);

    // Yukselisi SIMDIDEN kuyruga koy: mektup okunmasa da gerceklesecek.
    BeklemedeHaber bh;
    bh.hisseIndex = idx;
    bh.carpan = 1.0f + artis;
    bh.metin = "(insider)";
    bh.kalanSure = gecikme;
    o.bekleyenHaberler.push_back(bh);

    o.insiderMektupVar = true;
    o.insiderMektupOmur = INSIDER_MEKTUP_OMUR_SN;
    o.insiderOkundu = false;
    o.insiderHisseIndex = idx;
    o.insiderArtisOrani = artis;
    o.insiderKalanSure = gecikme;

    // Haber bandina BILEREK yazilmaz - bu bilgi sadece oyuncuya geliyor.
    o.toastlar.Ekle(M("GİZLİ ZARF", "SECRET ENVELOPE"),
                    M("Üst barda seni bekleyen bir mektup var.", "A letter is waiting for you in the top bar."),
                    RENK_ALTIN, 5.0f);
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
    if (Rastgele01() >= KURESEL_OLAY_OLASILIK) return;
    float iyiSans = ACEMI_KURESEL_IYI_TABAN + ACEMI_KURESEL_IYI_BONUS * AcemiKorumasi(o);
    const auto& sablon = KURESEL_OLAYLARI[AgirlikliOlaySec(
        KURESEL_OLAYLARI, iyiSans, [](const KureselOlaySablonu& k) { return k.temelCarpan; })];

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
    for (auto it = o.bekleyenKureselHaberler.begin(); it != o.bekleyenKureselHaberler.end();) {
        it->kalanSure -= dt;
        if (it->kalanSure <= 0.0f) {
            for (auto& h : o.hisseler) {
                // Etki her hissede kendi betasiyla olceklenir; vurgulanan sektor
                // ayrica (bazen TERS yonde) etkilenir.
                float etki = (it->carpan - 1.0f) * h.beta;
                if (it->vurgulananSektor != Sektor::YOK && h.sektor == it->vurgulananSektor)
                    etki += (it->sektorCarpani - 1.0f);
                h.fiyat = fmaxf(0.5f, h.fiyat * (1.0f + etki));
                h.temelDeger = fmaxf(0.5f, h.temelDeger * (1.0f + etki));
                h.volatilite = fminf(VOL_TAVAN, h.volatilite * VOL_SOK_CARPANI);
                h.gosterilenFiyat.Ayarla(h.fiyat);
                h.flashSuresi = 0.6f;
                h.flashIyi = etki >= 0.0f;
            }
            o.kureselRiskPrimi = Clamp(o.kureselRiskPrimi + it->riskPrimiDegisimi,
                                       KURESEL_RISK_PRIMI_TABANI, KURESEL_RISK_PRIMI_TAVANI);
            Cal(o.sesler.haberEtki);
            it = o.bekleyenKureselHaberler.erase(it);
        } else {
            ++it;
        }
    }
}
