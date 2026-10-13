// ============================================================================
// arayuz_grafik.h - ana grafik paneli (uc mod), gorev satiri, guvenli liman
//                   mini kartlari, rehber katmani ve YATAY oyun alaninin girdisi
// ============================================================================
// GrafikCiz iki yerlesimde ortaktir: a.grafikPanel nereye konduysa oraya cizer.
// Dikeyde (dar panel) baslik ve lejant cizilmez; lejantin isini varlik secici
// cipleri gorur.

// "GOREV: ...   odul X TL   N gun" - aktif gorev yoksa bos.
static string GorevSatiri(const Oyun& o) {
    if (o.gorevTur == G_YOK) return string();
    return string(M("GÖREV: ", "MISSION: ")) + GorevMetni(o);
}
static string GorevOdulMetni(const Oyun& o) {
    if (o.gorevTur == G_YOK) return string();
    return string(M("ödül ", "reward ")) + Para(o.gorevOdul, 0) + "  -  " + to_string(GorevKalanGun(o)) + M(" gün", "d");
}

// Guvenli liman mini kartlari (yatay; grafik panelinin en altinda).
static void LimanKartlariCiz(Uygulama& u) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    const int H = HisseSayisi();
    for (int k = 0; k < (int)LIMANLAR.size() && k < AZAMI_LIMAN; k++) {
        int i = H + k;
        if (i >= (int)o.hisseler.size()) break;
        Hisse& h = o.hisseler[i];
        Rectangle r = a.limanKart[k];
        PanelCiz(r, 0.2f, KartIcRengi(h), RENK_KENARLIK);
        DrawRectangleRounded({r.x, r.y, 6, r.height}, 0.0f, 4, h.renk);
        bool yukseldi = h.fiyat >= h.oncekiFiyat;
        string fiyat = Para(h.gosterilenFiyat.deger) + (yukseldi ? " ^" : " v");
        YaziCiz(h.kod, (int)r.x + 16, (int)r.y + 6, 14, WHITE);
        YaziCiz(fiyat, (int)r.x + 16, (int)r.y + 26, 13, yukseldi ? RENK_YESIL : RENK_KIRMIZI);

        // Orta sutun: elde tutulan adet ve kar/zarar (yer varsa)
        int ortaX = (int)r.x + 16 + max(YaziOlc(fiyat, 13), YaziOlc(h.kod, 14)) + 16;
        int ortaSag = (int)a.alBtn[i].rect.x - 10;
        string elde = string(M("Elde: ", "Held: ")) + to_string(h.adet);
        if (ortaX + YaziOlc(elde, 11) <= ortaSag) {
            YaziCiz(elde, ortaX, (int)r.y + 8, 11, RENK_SOLUK_METIN);
            float kzT = 0, kzO = 0;
            if (PozisyonKarZarari(h, kzT, kzO)) YaziCiz(Yuzde(kzO, 1, true), ortaX, (int)r.y + 26, 11, kzT >= 0 ? RENK_YESIL : RENK_KIRMIZI);
            else {
                string bas = BaskiMetni(h);
                if (!bas.empty() && ortaX + YaziOlc(bas, 11) <= ortaSag) YaziCiz(bas, ortaX, (int)r.y + 26, 11, RENK_UYARI);
            }
        }
        a.alBtn[i].Ciz(13);
        a.satBtn[i].Ciz(13);
    }
}

static void GrafikCiz(Uygulama& u, const KartDuzeni& kd) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    const bool dar = g_dikey;
    Rectangle gp = a.grafikPanel;
    PanelCiz(gp, dar ? 0.05f : 0.03f, RENK_PANEL, RENK_KENARLIK);
    const int sagKenar = (int)(gp.x + gp.width) - (dar ? 12 : 20);

    if (!dar) YaziCiz(M("CANLI PİYASA TAKİBİ", "LIVE MARKET"), (int)gp.x + 20, (int)gp.y + 16, 18, RENK_METIN);
    a.grafikModBtn.Ciz(dar ? 13 : 12);

    // "Piyasa Modu": ortak piyasa faktorunun yonu (hisselerin su an cogunlukla
    // birlikte hangi yone egilimli oldugu). Dar panelde kisa hali.
    {
        const char* mod;
        Color renk;
        if (o.piyasaFaktoru > PIYASA_MODU_ESIGI) {
            mod = dar ? M("PİYASA: YÜKSELİŞ", "MARKET: RISK-ON")
                      : M("PİYASA MODU: RISK-ON (Yükseliş Eğilimi)", "MARKET MODE: RISK-ON (Uptrend)");
            renk = RENK_YESIL;
        } else if (o.piyasaFaktoru < -PIYASA_MODU_ESIGI) {
            mod = dar ? M("PİYASA: DÜŞÜŞ", "MARKET: RISK-OFF")
                      : M("PİYASA MODU: RISK-OFF (Düşüş Eğilimi)", "MARKET MODE: RISK-OFF (Downtrend)");
            renk = RENK_KIRMIZI;
        } else {
            mod = dar ? M("PİYASA: NÖTR", "MARKET: NEUTRAL") : M("PİYASA MODU: NÖTR", "MARKET MODE: NEUTRAL");
            renk = RENK_SOLUK_METIN;
        }
        YaziCizSaga(mod, sagKenar, (int)gp.y + (dar ? 17 : 20), 13, renk);
    }

    // ---- ikinci satir(lar): gorev (solda) + iceriden bilgi seridi (sagda) ----
    // Hepsi tek satira sigiyorsa tek satir. Sigmiyorsa ALT ALTA dizilir ve cizim
    // alani o kadar asagi iner ("ek") - gorevin ne istedigi ve odulu HICBIR
    // durumda kirpilmaz. Dar panelde gorev metni odulle ayni satira sigmayabilir;
    // o zaman "GOREV + odul" bir satir, metnin kendisi bir alt satir olur.
    const int satir2Y = (int)gp.y + (dar ? 46 : 42);
    const int satirAralik = 24;
    const int gx = (int)gp.x + (dar ? 12 : 20);
    const int tamW = sagKenar - gx;
    int ek = 0;
    {
        const bool ipucu = o.insiderOkundu && o.insiderHisseIndex >= 0;
        string ins;
        int insW = 0;
        if (ipucu) {
            const Hisse& hi = o.hisseler[o.insiderHisseIndex];
            ins = string(dar ? M("İPUCU: ", "TIP: ") : M("İÇERİDEN BİLGİ:  ", "INSIDER TIP:  ")) + hi.kod + "  " +
                  Yuzde(o.insiderArtisOrani * 100.0f, 1, true) + "  " + SayiFormatla(fmaxf(0.0f, o.insiderKalanSure), 1) +
                  M(" sn", " s");
            insW = YaziOlc(ins, 13) + 20;
        }
        auto ipucuCiz = [&](int y) {
            Rectangle kutu = {(float)(sagKenar - insW), (float)y - 4, (float)insW, 22};
            DrawRectangleRounded(kutu, 0.45f, 6, Fade(RENK_ALTIN, 0.16f));
            DrawRectangleRounded({kutu.x, kutu.y, 4, kutu.height}, 0.9f, 4, RENK_ALTIN);
            YaziCiz(ins, (int)kutu.x + 12, (int)kutu.y + 5, 13, RENK_ALTIN);
        };

        string gorev = GorevSatiri(o);
        if (gorev.empty()) {
            if (ipucu) ipucuCiz(satir2Y);
            if (!dar) {
                string sure = string(M("(Son ", "(Last ")) + to_string(GECMIS_UZUNLUK) + M(" saniye)", " seconds)");
                YaziCiz(sure, gx, satir2Y, 12, RENK_SOLUK_METIN);
            }
        } else {
            string odul = GorevOdulMetni(o);
            const int gorevW = YaziOlc(gorev, 13), odulW = YaziOlc(odul, 12);
            const int ipucuPay = ipucu ? insW + 14 : 0;
            if (gorevW + 16 + odulW + ipucuPay <= tamW) {
                // hepsi tek satirda
                YaziCiz(gorev, gx, satir2Y, 13, RENK_METIN);
                YaziCizSaga(odul, sagKenar - ipucuPay, satir2Y + 1, 12, RENK_ALTIN);
                if (ipucu) ipucuCiz(satir2Y);
            } else if (gorevW + 16 + odulW <= tamW) {
                // gorev + odul ustte, ipucu bir alt satirda
                YaziCiz(gorev, gx, satir2Y, 13, RENK_METIN);
                YaziCizSaga(odul, sagKenar, satir2Y + 1, 12, RENK_ALTIN);
                ipucuCiz(satir2Y + satirAralik);
                ek = satirAralik;
            } else {
                // dar panel: "GOREV" + odul ustte, gorevin metni altta
                YaziCiz(M("GÖREV", "MISSION"), gx, satir2Y, 13, RENK_ALTIN);
                YaziCizSaga(odul, sagKenar, satir2Y + 1, 12, RENK_ALTIN);
                YaziCiz(Kisalt(GorevMetni(o), 13, tamW), gx, satir2Y + 20, 13, RENK_METIN);
                ek = 22;
                if (ipucu) {
                    ipucuCiz(satir2Y + 22 + satirAralik);
                    ek += satirAralik;
                }
            }
        }
    }

    // ---- cizim alani ----
    const bool limanSeridi = !dar && o.LimanlarAcik();
    Rectangle ca;
    if (dar) ca = {gp.x + 50, gp.y + 74 + ek, gp.width - 70, gp.height - 74 - 14 - ek};
    else ca = {gp.x + 60, gp.y + 70 + ek, gp.width - 100, gp.height - 140 - (limanSeridi ? 56.0f : 0.0f) - ek};
    if (ca.height < 40.0f) ca.height = 40.0f;

    // Cizilecek seriler. Mod 0 (TL): acik hisseler. Mod 1 (%): acik hisseler +
    // (aciksa) guvenli limanlar; yuzde ekseninde 3000 TL'lik altin da 38 TL'lik
    // hisse de ayni olcekte durur. Mod 2: net deger gecmisi (gunde bir nokta).
    vector<int> seriler = kd.acik;
    if (o.grafikModu == 1 && o.LimanlarAcik())
        for (int i = HisseSayisi(); i < (int)o.hisseler.size(); i++) seriler.push_back(i);

    auto grafikDegeri = [&](const Hisse& hh, float v) -> float {
        if (o.grafikModu != 1) return v;
        float taban = hh.gecmis.empty() ? v : hh.gecmis.front();
        if (taban < 0.01f) return 0.0f;
        return (v / taban - 1.0f) * 100.0f;
    };

    vector<float> netSeri;
    float minV = 1e18f, maxV = -1e18f;
    float hedefNet = -1.0f; // mod 2: siradaki rutbe esigi (aralikta ise kesikli cizgi)
    if (o.grafikModu == 2) {
        netSeri = o.netGecmisi;
        netSeri.push_back(o.NetDeger());
        for (float v : netSeri) { minV = fminf(minV, v); maxV = fmaxf(maxV, v); }
        if (o.rutbeIndex + 1 < (int)RUTBELER.size()) hedefNet = RUTBELER[o.rutbeIndex + 1].esikNetDeger;
        if (hedefNet > 0.0f && hedefNet < maxV * 1.6f) maxV = fmaxf(maxV, hedefNet);
        float pay = fmaxf(50.0f, (maxV - minV) * 0.08f);
        minV = fmaxf(0.0f, minV - pay);
        maxV += pay;
    } else {
        for (int i : seriler) {
            const Hisse& hh = o.hisseler[i];
            for (float v : hh.gecmis) {
                float d = grafikDegeri(hh, v);
                minV = fminf(minV, d);
                maxV = fmaxf(maxV, d);
            }
        }
        if (minV > maxV) { minV = 0.0f; maxV = 1.0f; }
        float esikFark = (o.grafikModu == 1) ? 0.5f : 1.0f;
        if (maxV - minV < esikFark) { minV -= esikFark * 5.0f; maxV += esikFark * 5.0f; }
    }

    for (int g = 0; g <= 4; g++) {
        float y = ca.y + ca.height * g / 4.0f;
        DrawLine((int)ca.x, (int)y, (int)(ca.x + ca.width), (int)y, Fade(RENK_KENARLIK, 0.6f));
        float deger = maxV - (maxV - minV) * g / 4.0f;
        string eksen = (o.grafikModu == 1) ? Yuzde(deger, 1, true) : SayiFormatla(deger, 0);
        YaziCizSaga(eksen, (int)ca.x - 6, (int)y - 6, 11, RENK_SOLUK_METIN);
    }

    if (o.grafikModu == 2) {
        // Siradaki rutbe esigi
        if (hedefNet > minV && hedefNet < maxV) {
            float y = ca.y + ca.height - (hedefNet - minV) / (maxV - minV) * ca.height;
            KesikliYatayCizgi(ca.x, ca.x + ca.width, y, Fade(RENK_ALTIN, 0.6f));
        }
        vector<Vector2> noktalar;
        int n = (int)netSeri.size();
        for (int k = 0; k < n; k++) {
            float t = (n <= 1) ? 0.0f : (float)k / (float)(n - 1);
            float oran = (netSeri[k] - minV) / (maxV - minV);
            noktalar.push_back({ca.x + t * ca.width, ca.y + ca.height - oran * ca.height});
        }
        if (n == 1) noktalar.push_back({ca.x + ca.width, noktalar[0].y});
        GlowCizgiCiz(noktalar, RENK_ALTIN, 2.4f);
    } else {
        // Bekleyen emir seviyeleri: varligin renginde kesikli cizgi + sagda harf.
        if (o.EmirlerAcik()) {
            for (int i : seriler) {
                const Hisse& h = o.hisseler[i];
                for (int t = 0; t < EMIR_TUR_SAYISI; t++) {
                    if (!h.emirler[t].aktif) continue;
                    float d = grafikDegeri(h, h.emirler[t].fiyat);
                    if (d < minV || d > maxV) continue;
                    float y = ca.y + ca.height - (d - minV) / (maxV - minV) * ca.height;
                    KesikliYatayCizgi(ca.x, ca.x + ca.width, y, Fade(h.renk, 0.55f));
                    const char* harf = (t == EMIR_STOP) ? "S" : (t == EMIR_KAR_AL ? M("K", "T") : "L");
                    TestKaydiDurdur(); // konumu veriye bagli; iki emir yakin olabilir
                    YaziCiz(harf, (int)(ca.x + ca.width) + 5, (int)y - 5, 10, h.renk);
                    TestKaydiSurdur();
                }
            }
        }
        for (int i : seriler) {
            Hisse& h = o.hisseler[i];
            vector<Vector2> noktalar;
            int n = (int)h.gecmis.size();
            for (int k = 0; k < n; k++) {
                float t = (n <= 1) ? 0.0f : (float)k / (float)(n - 1);
                float oran = (grafikDegeri(h, h.gecmis[k]) - minV) / (maxV - minV);
                noktalar.push_back({ca.x + t * ca.width, ca.y + ca.height - oran * ca.height});
            }
            // Dikeyde secili varlik kalin, digerleri ince cizilir.
            bool vurgulu = !dar || i == a.seciliVarlik;
            GlowCizgiCiz(noktalar, vurgulu ? h.renk : Fade(h.renk, 0.55f), h.liman ? 1.6f : (vurgulu ? 2.2f : 1.4f));
        }
    }

    // ---- lejant (yalnizca yatay) ----
    if (!dar) {
        float lx = gp.x + 20, ly = gp.y + gp.height - 40 - (limanSeridi ? 56.0f : 0.0f);
        if (o.grafikModu == 2) {
            DrawRectangleRounded({lx, ly, 14, 14}, 0.4f, 4, RENK_ALTIN);
            string ad = string(M("Net değer (gün 1 - ", "Net worth (day 1 - ")) + to_string(o.gun) + ")";
            YaziCiz(ad, (int)lx + 20, (int)ly - 1, 14, RENK_METIN);
            lx += 20 + (float)YaziOlc(ad, 14) + 24;
            if (hedefNet > 0.0f) {
                KesikliYatayCizgi(lx, lx + 28, ly + 7, RENK_ALTIN);
                YaziCiz(string(M("sıradaki rütbe: ", "next rank: ")) + Para(hedefNet, 0), (int)lx + 36, (int)ly, 13, RENK_SOLUK_METIN);
            }
        } else {
            for (int i : seriler) {
                Hisse& h = o.hisseler[i];
                DrawRectangleRounded({lx, ly, 14, 14}, 0.4f, 4, h.renk);
                YaziCiz(h.kod, (int)lx + 20, (int)ly - 1, 14, RENK_METIN);
                lx += 20 + (float)YaziOlc(h.kod, 14) + 24;
            }
        }
        if (limanSeridi) LimanKartlariCiz(u);
    }
}

// ---------------------------------------------------------------------------
// REHBER KATMANI
// ---------------------------------------------------------------------------
static Rectangle RehberHedefRect(const Arayuz& a, const KartDuzeni& kd, RehberHedef hedef) {
    if (g_dikey) {
        switch (hedef) {
            case RehberHedef::AL_BUTONU:
                if (a.seciliVarlik >= 0 && a.seciliVarlik < (int)a.alBtn.size()) return a.alBtn[a.seciliVarlik].rect;
                break;
            case RehberHedef::HABER_BANDI: return {a.haberBandi.x + 4, a.haberBandi.y + 2, a.haberBandi.width - 8, a.haberBandi.height - 4};
            case RehberHedef::KART:
            case RehberHedef::ETIKET: return a.dKart;
            case RehberHedef::RUTBE_CUBUGU: return {6, a.rutbeCubugu.y - 12, (float)PENCERE_GENISLIK - 12, 32};
            case RehberHedef::ADET_PANELI: return a.adetPanel;
            default: break;
        }
        return {0, 0, 0, 0};
    }
    switch (hedef) {
        case RehberHedef::AL_BUTONU:
            if (kd.sayi > 0) return a.alBtn[kd.acik[0]].rect;
            break;
        case RehberHedef::HABER_BANDI: return {a.haberBandi.x + 4, a.haberBandi.y + 2, a.haberBandi.width - 8, a.haberBandi.height - 4};
        case RehberHedef::KART:
            if (kd.sayi > 0) return KartRect(a, kd, 0);
            break;
        case RehberHedef::ETIKET:
            if (kd.sayi > 0) {
                Rectangle k = KartRect(a, kd, 0);
                return {k.x + 10, k.y + 4, 200, 30};
            }
            break;
        case RehberHedef::RUTBE_CUBUGU: return {a.rutbeCubugu.x - 8, 38, 420, 52};
        case RehberHedef::ADET_PANELI: return a.adetPanel;
        default: break;
    }
    return {0, 0, 0, 0};
}

static Vector2 EnYakinNokta(Rectangle r, Vector2 p) {
    return {Clamp(p.x, r.x, r.x + r.width), Clamp(p.y, r.y, r.y + r.height)};
}

static void RehberCiz(Uygulama& u, const KartDuzeni& kd) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (o.rehberAdim < 0 || o.rehberAdim >= (int)REHBER_ADIMLARI.size() || o.rehberBekleme > 0.0f) return;
    const RehberAdimi& adim = REHBER_ADIMLARI[o.rehberAdim];
    Rectangle kutu = RehberKutusuHesapla(a, o);
    Rectangle hedef = RehberHedefRect(a, kd, adim.hedef);

    if (hedef.width > 0.0f) {
        float nabiz = 3.0f + sinf(u.zaman * 5.0f) * 2.0f;
        Rectangle cerceve = {hedef.x - nabiz, hedef.y - nabiz, hedef.width + nabiz * 2, hedef.height + nabiz * 2};
        DrawRectangleLinesEx(cerceve, 3.0f, RENK_ALTIN);
        Vector2 kutuMerkez = {kutu.x + kutu.width / 2, kutu.y + kutu.height / 2};
        Vector2 hedefMerkez = {hedef.x + hedef.width / 2, hedef.y + hedef.height / 2};
        Vector2 p1 = EnYakinNokta(kutu, hedefMerkez);
        Vector2 p2 = EnYakinNokta(cerceve, kutuMerkez);
        DrawLineEx(p1, p2, 3.0f, Fade(RENK_ALTIN, 0.85f));
        DrawCircleV(p2, 6.0f, RENK_ALTIN);
    }

    KatmanBaslat();
    DrawRectangleRounded({kutu.x + 4, kutu.y + 5, kutu.width, kutu.height}, 0.08f, 8, Fade(BLACK, 0.45f));
    PanelCiz(kutu, 0.08f, RENK_PANEL_ACIK, RENK_ALTIN);
    string baslik = string(M("REHBER  ", "GUIDE  ")) + to_string(o.rehberAdim + 1) + " / " + to_string((int)REHBER_ADIMLARI.size());
    YaziCiz(baslik, (int)kutu.x + 18, (int)kutu.y + 14, 13, RENK_ALTIN);
    YaziCizSarili(adim.metin.al(), (int)kutu.x + 18, (int)kutu.y + 38, 14, (int)kutu.width - 36, RENK_METIN, 0, 19);
    a.rehberTamamBtn.Ciz(14);
    a.rehberKapatBtn.Ciz(13);
}

// Rehber kutusunun dugmeleri. Bir tik rehber tarafindan "yenildiyse" true doner
// (alttaki bir seyi tetiklemesin diye).
static bool RehberGirdisi(Uygulama& u, float dt, Vector2 fare, bool tik) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (o.rehberAdim < 0 || o.rehberBekleme > 0.0f) return false;
    Rectangle kutu = RehberKutusuHesapla(a, o);
    if (a.rehberTamamBtn.Guncelle(dt, fare, tik)) {
        RehberIlerle(o);
        Cal(o.sesler.tikla);
        return true;
    }
    if (a.rehberKapatBtn.Guncelle(dt, fare, tik)) {
        RehberiBitir(o);
        Cal(o.sesler.tikla);
        return true;
    }
    return tik && CheckCollisionPointRec(fare, kutu);
}

// Bir varligin AL / SAT / EMIR dugmelerini isler (yatay kartlar, liman kartlari
// ve dikey kart ayni kodu kullanir).
static void VarlikDugmeleriniIsle(Uygulama& u, int i, float dt, Vector2 fare, bool tik, bool emirDugmesi) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    if (a.alBtn[i].Guncelle(dt, fare, tik)) {
        if (HisseAl(o, i)) {
            Rectangle r = a.alBtn[i].rect;
            o.parcaciklar.Patlat({r.x + r.width / 2, r.y}, RENK_YESIL, 16, 180.0f);
        }
    }
    if (a.satBtn[i].Guncelle(dt, fare, tik)) {
        if (HisseSat(o, i)) {
            Rectangle r = a.satBtn[i].rect;
            o.parcaciklar.Patlat({r.x + r.width / 2, r.y}, RENK_ALTIN, 16, 180.0f);
        }
    }
    if (emirDugmesi && a.emirBtn[i].Guncelle(dt, fare, tik)) {
        a.emirHisse = i;
        Cal(o.sesler.tikla);
    }
}

// Iki yerlesimde ortak: adet paneli, grafik modu, kredi dugmesi, zarf.
static void OrtakOyunGirdisi(Uygulama& u, float dt, const Girdi& g, Vector2 fare, bool tik, bool etkin) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    for (int i = 0; i < 8; i++) {
        if (a.adetBtn[i].Guncelle(dt, fare, tik)) {
            o.secilenAdet = a.adetDegerleri[i];
            Cal(o.sesler.tikla);
        }
    }
    if (etkin && g.tusAdet >= 0 && g.tusAdet < 8) {
        o.secilenAdet = a.adetDegerleri[g.tusAdet];
        Cal(o.sesler.tikla);
    }
    if (a.grafikModBtn.Guncelle(dt, fare, tik)) {
        o.grafikModu = (o.grafikModu + 1) % 3;
        Cal(o.sesler.tikla);
    }
    if (o.KrediAcik() && a.krediBtn.Guncelle(dt, fare, tik)) {
        a.krediAcik = true;
        Cal(o.sesler.tikla);
    }
    if (o.insiderMektupVar && tik && CheckCollisionPointRec(fare, a.insiderRect)) {
        InsiderMektubuAc(o);
        o.parcaciklar.Patlat({a.insiderRect.x + a.insiderRect.width / 2.0f, a.insiderRect.y + a.insiderRect.height / 2.0f},
                             RENK_ALTIN, 24, 200.0f);
    }
}

// ---------------------------------------------------------------------------
// YATAY OYUN ALANININ GIRDISI
// ---------------------------------------------------------------------------
// etkin=false iken (duraklatma / acik bir pencere) hicbir sey tiklanamaz; dugmeler
// sadece "uzerinde degil" durumuna yumusakca doner.
static void OyunAlaniGuncelle(Uygulama& u, float dt, const Girdi& g, const KartDuzeni& kd, bool etkin) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    Vector2 fare = etkin ? g.fare : FARE_UZAK;
    bool tik = etkin && g.tiklandi;

    // Rehber kutusu grafik alaninin ustunde durur; once o tiki alir.
    if (RehberGirdisi(u, dt, fare, tik)) tik = false;

    for (int slot = 0; slot < kd.sayi; slot++) VarlikDugmeleriniIsle(u, kd.acik[slot], dt, fare, tik, o.EmirlerAcik());
    if (o.LimanlarAcik())
        for (int i = HisseSayisi(); i < (int)o.hisseler.size(); i++) VarlikDugmeleriniIsle(u, i, dt, fare, tik, false);

    OrtakOyunGirdisi(u, dt, g, fare, tik, etkin);

    // "Yeni Oyun": iki asamali onay (ilk tik "Emin misin?" yapar; SIFIRLA_ONAY_SN
    // icinde gelen ikinci tik ayni senaryoyu bastan baslatir). Duraklatmada ve
    // pencereler acikken tiklanamaz - karartmanin altindaki dugmeye basip oyunu
    // sessizce sifirlamak mumkun olmasin.
    if (a.sifirlaOnaySuresi > 0.0f) a.sifirlaOnaySuresi = fmaxf(0.0f, a.sifirlaOnaySuresi - dt);
    if (a.sifirlaBtn.Guncelle(dt, fare, tik)) {
        if (a.sifirlaOnaySuresi > 0.0f) {
            o.Baslat(o.senaryo, false);
            a.sifirlaOnaySuresi = 0.0f;
            Cal(o.sesler.tikla);
        } else {
            a.sifirlaOnaySuresi = SIFIRLA_ONAY_SN;
            Cal(o.sesler.hataUyari);
        }
    }
}
