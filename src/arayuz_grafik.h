// ============================================================================
// arayuz_grafik.h - ana grafik paneli, rehber katmani ve oyun alaninin girdisi
// ============================================================================

static void GrafikCiz(Uygulama& u, const KartDuzeni& kd) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    Rectangle gp = a.grafikPanel;
    PanelCiz(gp, 0.03f, RENK_PANEL, RENK_KENARLIK);
    YaziCiz(M("CANLI PİYASA TAKİBİ", "LIVE MARKET"), (int)gp.x + 20, (int)gp.y + 16, 18, RENK_METIN);
    a.grafikModBtn.Ciz(12);
    string sure = string(M("(Son ", "(Last ")) + to_string(GECMIS_UZUNLUK) + M(" saniye)", " seconds)");
    YaziCiz(sure, (int)gp.x + 20, (int)gp.y + 40, 12, RENK_SOLUK_METIN);

    // "Piyasa Modu": ortak piyasa faktorunun yonu (hisselerin su an cogunlukla
    // birlikte hangi yone egilimli oldugu).
    {
        const char* mod;
        Color renk;
        if (o.piyasaFaktoru > PIYASA_MODU_ESIGI) {
            mod = M("PİYASA MODU: RISK-ON (Yükseliş Eğilimi)", "MARKET MODE: RISK-ON (Uptrend)");
            renk = RENK_YESIL;
        } else if (o.piyasaFaktoru < -PIYASA_MODU_ESIGI) {
            mod = M("PİYASA MODU: RISK-OFF (Düşüş Eğilimi)", "MARKET MODE: RISK-OFF (Downtrend)");
            renk = RENK_KIRMIZI;
        } else {
            mod = M("PİYASA MODU: NÖTR", "MARKET MODE: NEUTRAL");
            renk = RENK_SOLUK_METIN;
        }
        YaziCizSaga(mod, (int)(gp.x + gp.width) - 20, (int)gp.y + 20, 13, renk);
    }

    // Iceriden bilgi okunduktan sonra, yukselis gerceklesene kadar duran serit.
    if (o.insiderOkundu && o.insiderHisseIndex >= 0) {
        Hisse& hi = o.hisseler[o.insiderHisseIndex];
        string ins = string(M("İÇERİDEN BİLGİ:  ", "INSIDER TIP:  ")) + hi.kod + "   " +
                     Yuzde(o.insiderArtisOrani * 100.0f, 1, true) + "   " +
                     SayiFormatla(fmaxf(0.0f, o.insiderKalanSure), 1) + M(" sn", " s");
        int iw = YaziOlc(ins, 13);
        Rectangle kutu = {gp.x + gp.width - 20 - (float)iw - 20, gp.y + 40, (float)iw + 20, 22};
        DrawRectangleRounded(kutu, 0.45f, 6, Fade(RENK_ALTIN, 0.16f));
        DrawRectangleRounded({kutu.x, kutu.y, 4, kutu.height}, 0.9f, 4, RENK_ALTIN);
        YaziCiz(ins, (int)kutu.x + 12, (int)kutu.y + 5, 13, RENK_ALTIN);
    }

    Rectangle ca = {gp.x + 60, gp.y + 70, gp.width - 100, gp.height - 140};

    // NORMALIZE MODDA her hisse kendi gecmis penceresinin ILK degerine gore yuzde
    // degisim olarak cizilir; 430 TL'lik BIMAS ile 38 TL'lik EREGL ayni olcekte
    // karsilastirilabilir (mutlak eksende ucuz hisseler duz cizgiye doner).
    auto grafikDegeri = [&](const Hisse& hh, float v) -> float {
        if (!o.normalizeGrafik) return v;
        float taban = hh.gecmis.empty() ? v : hh.gecmis.front();
        if (taban < 0.01f) return 0.0f;
        return (v / taban - 1.0f) * 100.0f;
    };
    float minV = 1e9f, maxV = -1e9f;
    for (int i : kd.acik) {
        const Hisse& hh = o.hisseler[i];
        for (float v : hh.gecmis) {
            float d = grafikDegeri(hh, v);
            minV = fminf(minV, d);
            maxV = fmaxf(maxV, d);
        }
    }
    if (minV > maxV) { minV = 0.0f; maxV = 1.0f; }
    float esikFark = o.normalizeGrafik ? 0.5f : 1.0f;
    if (maxV - minV < esikFark) { minV -= esikFark * 5.0f; maxV += esikFark * 5.0f; }

    for (int g = 0; g <= 4; g++) {
        float y = ca.y + ca.height * g / 4.0f;
        DrawLine((int)ca.x, (int)y, (int)(ca.x + ca.width), (int)y, Fade(RENK_KENARLIK, 0.6f));
        float deger = maxV - (maxV - minV) * g / 4.0f;
        string eksen = o.normalizeGrafik ? Yuzde(deger, 1, true) : SayiFormatla(deger, 0);
        YaziCizSaga(eksen, (int)ca.x - 8, (int)y - 6, 11, RENK_SOLUK_METIN);
    }

    // Bekleyen emir seviyeleri: hissenin renginde kesikli cizgi + sagda harf.
    if (o.EmirlerAcik()) {
        for (int i : kd.acik) {
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

    for (int i : kd.acik) {
        Hisse& h = o.hisseler[i];
        vector<Vector2> noktalar;
        int n = (int)h.gecmis.size();
        for (int k = 0; k < n; k++) {
            float t = (n <= 1) ? 0.0f : (float)k / (float)(n - 1);
            float oran = (grafikDegeri(h, h.gecmis[k]) - minV) / (maxV - minV);
            noktalar.push_back({ca.x + t * ca.width, ca.y + ca.height - oran * ca.height});
        }
        GlowCizgiCiz(noktalar, h.renk, 2.2f);
    }

    // Lejant (sadece acik hisseler)
    float lx = gp.x + 20, ly = gp.y + gp.height - 40;
    for (int i : kd.acik) {
        Hisse& h = o.hisseler[i];
        DrawRectangleRounded({lx, ly, 14, 14}, 0.4f, 4, h.renk);
        YaziCiz(h.kod, (int)lx + 20, (int)ly - 1, 14, RENK_METIN);
        lx += 20 + (float)YaziOlc(h.kod, 14) + 24;
    }
}

// ---------------------------------------------------------------------------
// REHBER KATMANI
// ---------------------------------------------------------------------------
static Rectangle RehberHedefRect(const Arayuz& a, const KartDuzeni& kd, RehberHedef hedef) {
    switch (hedef) {
        case RehberHedef::AL_BUTONU:
            if (kd.sayi > 0) return a.alBtn[kd.acik[0]].rect;
            break;
        case RehberHedef::HABER_BANDI: return {a.haberBandi.x + 4, a.haberBandi.y + 2, a.haberBandi.width - 8, a.haberBandi.height - 4};
        case RehberHedef::KART:
            if (kd.sayi > 0) return KartRect(a, kd, 0);
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
    YaziCizSarili(adim.metin.al(), (int)kutu.x + 18, (int)kutu.y + 38, 14, REHBER_KUTU_W - 36, RENK_METIN, 0, 19);
    a.rehberTamamBtn.Ciz(14);
    a.rehberKapatBtn.Ciz(13);
}

// ---------------------------------------------------------------------------
// OYUN ALANININ GIRDISI (kartlar, adet paneli, ust bar dugmeleri, rehber)
// ---------------------------------------------------------------------------
// etkin=false iken (duraklatma / acik bir pencere) hicbir sey tiklanamaz; dugmeler
// sadece "uzerinde degil" durumuna yumusakca doner.
static void OyunAlaniGuncelle(Uygulama& u, float dt, const Girdi& g, const KartDuzeni& kd, bool etkin) {
    Oyun& o = u.oyun;
    Arayuz& a = u.ui;
    Vector2 fare = etkin ? g.fare : FARE_UZAK;
    bool tik = etkin && g.tiklandi;

    // Rehber kutusu grafik alaninin ustunde durur; once o tiki alir.
    if (o.rehberAdim >= 0 && o.rehberBekleme <= 0.0f) {
        Rectangle kutu = RehberKutusuHesapla(a, o);
        if (a.rehberTamamBtn.Guncelle(dt, fare, tik)) {
            RehberIlerle(o);
            Cal(o.sesler.tikla);
            tik = false;
        } else if (a.rehberKapatBtn.Guncelle(dt, fare, tik)) {
            RehberiBitir(o);
            Cal(o.sesler.tikla);
            tik = false;
        } else if (tik && CheckCollisionPointRec(fare, kutu)) {
            tik = false; // kutunun kendisine tiklamak alttaki bir seyi tetiklemesin
        }
    }

    for (int slot = 0; slot < kd.sayi; slot++) {
        int i = kd.acik[slot];
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
        if (o.EmirlerAcik() && a.emirBtn[i].Guncelle(dt, fare, tik)) {
            a.emirHisse = i;
            Cal(o.sesler.tikla);
        }
    }

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

    if (a.grafikModBtn.Guncelle(dt, fare, tik)) {
        o.normalizeGrafik = !o.normalizeGrafik;
        Cal(o.sesler.tikla);
    }

    if (o.KrediAcik()) {
        if (a.krediBtn.Guncelle(dt, fare, tik)) {
            a.krediAcik = true;
            Cal(o.sesler.tikla);
        }
    }

    // Iceriden bilgi zarfi
    if (o.insiderMektupVar && tik && CheckCollisionPointRec(fare, a.insiderRect)) {
        InsiderMektubuAc(o);
        o.parcaciklar.Patlat({a.insiderRect.x + a.insiderRect.width / 2.0f, a.insiderRect.y + a.insiderRect.height / 2.0f},
                             RENK_ALTIN, 24, 200.0f);
    }
}
