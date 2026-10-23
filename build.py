#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Borsa Patronu - derleme betigi (tek komutla her sey).

    python build.py           Windows .exe'yi derler ve dogrular
    python build.py test      denge testi + gorsel test (yerlesim denetcisi)
    python build.py web       tarayici surumunu derler (web_cikti/)
    python build.py chrome    Chrome Web Store uzantisi paketini yapar ve denetler
    python build.py magaza    magaza gorsellerini uretir (gercek oyundan ekran goruntuleri)
    python build.py video     tanitim videolari (Turkce + Ingilizce) -> release/video/
    python build.py paket     hepsini yapar, release/ altina zip'leri koyar
    python build.py sun       web surumunu http://localhost:8765 adresinde sunar
    python build.py yayinla   web surumunu derler ve GitHub Pages'e gonderir

Surum numarasi TEK yerden okunur: src/surum.h
Arac yollari asagidaki sabitlerde; baska bir makinede buralari duzelt.
"""
import hashlib
import json
import struct
import os
import re
import shutil
import subprocess
import sys
import tempfile
import zipfile

KOK = os.path.dirname(os.path.abspath(__file__))

# ---- arac yollari -----------------------------------------------------------
W64DEVKIT_ADAYLARI = [
    r"C:\Users\Lizer\Downloads\w64devkit\bin",  # GCC 16
    r"C:\raylib\w64devkit\bin",                 # raylib kurulumuyla gelen GCC 15
]
RAYLIB = "C:/raylib/raylib/src"
EMSDK = r"C:\raylib\emsdk"

ORTAK_BAYRAKLAR = ["-std=c++17", "-Wall", "-I", RAYLIB]
# -mwindows  : konsol penceresi ACILMAZ (bunsuz oyunun arkasinda siyah cmd cikar)
# -static*   : libgcc/libstdc++/winpthread .exe'ye gomulur (baska PC'de DLL aramaz)
EXE_BAGLAMA = ["-L", RAYLIB, "-lraylib", "-lopengl32", "-lgdi32", "-lwinmm",
               "-mwindows", "-static", "-static-libgcc", "-static-libstdc++"]
TEST_BAGLAMA = ["-L", RAYLIB, "-lraylib", "-lopengl32", "-lgdi32", "-lwinmm"]
SISTEM_DLLERI = {"kernel32.dll", "user32.dll", "gdi32.dll", "shell32.dll", "winmm.dll",
                 "msvcrt.dll", "opengl32.dll", "advapi32.dll", "ole32.dll"}


def hata(mesaj):
    print("\nHATA: " + mesaj)
    sys.exit(1)


def surum():
    with open(os.path.join(KOK, "src", "surum.h"), encoding="utf-8") as f:
        m = re.search(r'#define\s+SURUM_METIN\s+"([^"]+)"', f.read())
    if not m:
        hata("src/surum.h icinde SURUM_METIN bulunamadi")
    return m.group(1)


def ortam():
    """g++/windres/objdump PATH'te olacak sekilde bir ortam dondurur."""
    env = dict(os.environ)
    for aday in W64DEVKIT_ADAYLARI:
        if os.path.exists(os.path.join(aday, "g++.exe")):
            env["PATH"] = aday + os.pathsep + env.get("PATH", "")
            return env
    hata("w64devkit bulunamadi. build.py basindaki W64DEVKIT_ADAYLARI listesini duzelt.")


def calistir(komut, env=None, cwd=KOK, yakala=False):
    print("  > " + " ".join(os.path.basename(komut[0]) if i == 0 else k for i, k in enumerate(komut))[:160])
    # Windows, calistirilacak programi cocuk surecin degil BU surecin PATH'inde
    # arar; o yuzden tam yolu kendimiz cozuyoruz.
    if env is not None and not os.path.isabs(komut[0]):
        tam = shutil.which(komut[0], path=env.get("PATH"))
        if tam:
            komut = [tam] + list(komut[1:])
    sonuc = subprocess.run(komut, env=env, cwd=cwd, capture_output=yakala, text=yakala,
                           encoding="utf-8" if yakala else None, errors="replace" if yakala else None)
    return sonuc


def exe_derle():
    env = ortam()
    print("== Windows .exe (%s) ==" % surum())
    os.makedirs(os.path.join(KOK, "build"), exist_ok=True)
    res = os.path.join("build", "borsa_patronu_res.o")
    if calistir(["windres", "borsa_patronu.rc", "-O", "coff", "-o", res], env).returncode != 0:
        hata("windres basarisiz")
    cikti = "borsa_patronu.exe"
    komut = ["g++", "borsa_patronu.cpp", res, "-o", cikti, "-O2"] + ORTAK_BAYRAKLAR + EXE_BAGLAMA
    s = calistir(komut, env, yakala=True)
    if s.stdout.strip() or s.stderr.strip():
        print((s.stdout + s.stderr).strip())
    if s.returncode != 0:
        if "Permission denied" in s.stderr:
            hata("borsa_patronu.exe yazilamadi - oyun acik olabilir, kapatip tekrar dene.")
        hata("derleme basarisiz")
    if "warning" in s.stderr:
        hata("derleme UYARI verdi; proje sifir uyariyla derlenmeli.")

    # ---- dogrulamalar ----
    d = calistir(["objdump", "-p", cikti], env, yakala=True).stdout
    if "(Windows GUI)" not in d:
        hata(".exe konsol uygulamasi olarak derlendi (-mwindows eksik)")
    dller = {m.lower() for m in re.findall(r"DLL Name:\s*(\S+)", d)}
    yabanci = dller - SISTEM_DLLERI
    if yabanci:
        hata("exe sistem disi DLL istiyor (static bayraklari eksik?): " + ", ".join(sorted(yabanci)))
    boyut = os.path.getsize(os.path.join(KOK, cikti)) / 1e6
    print("  tamam: %s (%.1f MB), GUI alt sistemi, sadece sistem DLL'leri: %s" % (cikti, boyut, ", ".join(sorted(dller))))
    return cikti


def testleri_calistir():
    env = ortam()
    print("== testler ==")
    os.makedirs(os.path.join(KOK, "test", "cikti"), exist_ok=True)
    basarisiz = False
    for ad in ("denge_testi", "gorsel_test"):
        kaynak = os.path.join("test", ad + ".cpp")
        exe = os.path.join("test", ad + ".exe")
        komut = ["g++", kaynak, "-o", exe, "-O2", "-Wno-unused-function"] + ORTAK_BAYRAKLAR + TEST_BAGLAMA
        s = calistir(komut, env, yakala=True)
        if s.returncode != 0:
            print(s.stderr)
            hata(ad + " derlenemedi")
        s = calistir([os.path.join(KOK, exe)], env, yakala=True)
        # Uzun raporun sadece ozetini goster; tamami dosyada.
        satirlar = [x for x in s.stdout.splitlines() if x.strip()]
        ozet = [x for x in satirlar if not x.rstrip().endswith(" ok")]
        print("\n".join("    " + x for x in ozet[-40:]))
        if s.returncode != 0:
            basarisiz = True
    print("  raporlar: test/denge_raporu.txt, test/cikti/yerlesim_raporu.txt, test/cikti/*.png")
    if basarisiz:
        hata("testler BASARISIZ")
    print("  testler TEMIZ")


def web_derle():
    print("== web surumu (%s) ==" % surum())
    emcc = os.path.join(EMSDK, "upstream", "emscripten", "emcc.bat")
    weblib = os.path.join(RAYLIB, "libraylib.web.a")
    if not os.path.exists(emcc):
        hata("emscripten bulunamadi: " + emcc)
    if not os.path.exists(weblib):
        hata("raylib web kutuphanesi bulunamadi: " + weblib)
    env = dict(os.environ)
    env["EMSDK"] = EMSDK
    env["EM_CONFIG"] = os.path.join(EMSDK, ".emscripten")
    for alt in ("python", "node"):
        kok = os.path.join(EMSDK, alt)
        if os.path.isdir(kok):
            surumler = sorted(os.listdir(kok))
            if surumler:
                yol = os.path.join(kok, surumler[-1])
                if alt == "python":
                    env["EMSDK_PYTHON"] = os.path.join(yol, "python.exe")
                else:
                    env["EMSDK_NODE"] = os.path.join(yol, "bin", "node.exe")
                    env["PATH"] = os.path.join(yol, "bin") + os.pathsep + env["PATH"]
    cikti_dizin = os.path.join(KOK, "web_cikti")
    shutil.rmtree(cikti_dizin, ignore_errors=True)
    os.makedirs(cikti_dizin)
    komut = [emcc, "borsa_patronu.cpp", "-o", "web_cikti/index.html", "-O2", "-std=c++17", "-Wall",
             "-I", RAYLIB, weblib, "-DPLATFORM_WEB",
             "-sUSE_GLFW=3", "-sALLOW_MEMORY_GROWTH=1", "-sSTACK_SIZE=1048576",
             "-sEXPORTED_FUNCTIONS=_main,_malloc,_free",
             # eval / new Function YOK: Chrome uzantisinin guvenlik kurali (CSP) yasaklar.
             "-sDYNAMIC_EXECUTION=0",
             "--shell-file", "web/kabuk.html"]
    s = calistir(komut, env, yakala=True)
    cikti = (s.stdout + s.stderr).strip()
    if cikti:
        print(cikti[-3000:])
    if s.returncode != 0:
        hata("web derlemesi basarisiz")
    # Sayfanin ek dosyalari: yardimci betik, sekme simgesi ve paylasim onizleme gorseli.
    shutil.copyfile(os.path.join(KOK, "web", "kabuk.js"), os.path.join(cikti_dizin, "kabuk.js"))
    # Gizlilik politikasi (Chrome Web Store adresini ister): .../borsa-patronu/gizlilik.html
    shutil.copyfile(os.path.join(KOK, "web", "gizlilik.html"), os.path.join(cikti_dizin, "gizlilik.html"))
    shutil.copyfile(os.path.join(KOK, "borsa_patronu.ico"), os.path.join(cikti_dizin, "favicon.ico"))
    kapak = os.path.join(KOK, "sayfa_gorselleri", "kapak_630x500.png")
    if os.path.exists(kapak):
        shutil.copyfile(kapak, os.path.join(cikti_dizin, "kapak.png"))
    dosyalar = sorted(os.listdir(cikti_dizin))
    toplam = sum(os.path.getsize(os.path.join(cikti_dizin, d)) for d in dosyalar) / 1e6
    print("  tamam: web_cikti/ (%s) toplam %.1f MB" % (", ".join(dosyalar), toplam))
    return cikti_dizin


def ico_pngleri(yol):
    """borsa_patronu.ico icindeki PNG girdilerini {boyut: bayt} olarak doner."""
    with open(yol, "rb") as f:
        veri = f.read()
    _, tip, adet = struct.unpack_from("<HHH", veri, 0)
    if tip != 1:
        hata("ikon dosyasi ICO degil: " + yol)
    sonuc = {}
    for i in range(adet):
        w, h, _, _, _, _, uzunluk, ofs = struct.unpack_from("<BBBBHHII", veri, 6 + 16 * i)
        parca = veri[ofs:ofs + uzunluk]
        if parca[:8] == b"\x89PNG\r\n\x1a\n":
            sonuc[w or 256] = parca
    return sonuc


def png_boyutu(veri):
    return struct.unpack(">II", veri[16:24])


def chrome_paketle():
    """Chrome Web Store'a yuklenecek uzanti paketini hazirlar ve DENETLER.

    Chrome Web Store artik "uygulama" kabul etmiyor; oyun bir UZANTI olarak
    yayinlanir: arac cubugundaki simgeye tiklayinca oyun yeni sekmede acilir.
    Oyunun her dosyasi pakettedir (Manifest V3 uzaktan kod yuklemeyi yasaklar),
    hicbir izin istenmez. Kayitlar uzantinin kendi localStorage alaninda durur.
    """
    web = web_derle()
    s = surum()
    print("== Chrome uzantisi (%s) ==" % s)
    hedef = os.path.join(KOK, "build", "chrome")
    shutil.rmtree(hedef, ignore_errors=True)
    os.makedirs(os.path.join(hedef, "simge"))

    # 1) Oyunun kendisi (web derlemesi). Paylasim onizleme gorseli ve gizlilik
    #    sayfasi (web sitesinde duruyor) uzantida gereksiz.
    for ad in os.listdir(web):
        if ad not in ("kapak.png", "gizlilik.html"):
            shutil.copyfile(os.path.join(web, ad), os.path.join(hedef, ad))
    # 2) Uzantiya ozel dosyalar
    shutil.copyfile(os.path.join(KOK, "chrome", "arkaplan.js"), os.path.join(hedef, "arkaplan.js"))
    shutil.copytree(os.path.join(KOK, "chrome", "_locales"), os.path.join(hedef, "_locales"))
    # 3) Simgeler: .exe ikonuyla AYNI cizim (ikon_uret.py), .ico icinden cikarilir
    pngler = ico_pngleri(os.path.join(KOK, "borsa_patronu.ico"))
    for n in (16, 32, 48, 128):
        if n not in pngler:
            hata("ikon dosyasinda %dpx yok (ikon_uret.py'yi calistir)" % n)
        with open(os.path.join(hedef, "simge", "%d.png" % n), "wb") as f:
            f.write(pngler[n])
    # 4) Manifest: surum src/surum.h'den ("v2.1" -> "2.1.0")
    with open(os.path.join(KOK, "chrome", "manifest.json"), encoding="utf-8") as f:
        manifest = json.load(f)
    sayilar = re.findall(r"\d+", s)
    manifest["version"] = ".".join((sayilar + ["0", "0"])[:3])
    manifest["version_name"] = s
    with open(os.path.join(hedef, "manifest.json"), "w", encoding="utf-8", newline="\n") as f:
        json.dump(manifest, f, ensure_ascii=False, indent=2)

    # ---- DENETIMLER (magaza reddetmeden once burada yakala) ----
    sorunlar = []
    def dosya_var(yol):
        if not os.path.exists(os.path.join(hedef, yol)):
            sorunlar.append("manifestin andigi dosya yok: " + yol)
    for yol in list(manifest["icons"].values()) + list(manifest["action"]["default_icon"].values()):
        dosya_var(yol)
    dosya_var(manifest["background"]["service_worker"])
    for boy, yol in manifest["icons"].items():
        tam = os.path.join(hedef, yol)
        if os.path.exists(tam):
            with open(tam, "rb") as f:
                w, h = png_boyutu(f.read())
            if (w, h) != (int(boy), int(boy)):
                sorunlar.append("%s %dx%d, %s olmali" % (yol, w, h, boy))
    for dil in os.listdir(os.path.join(hedef, "_locales")):
        with open(os.path.join(hedef, "_locales", dil, "messages.json"), encoding="utf-8") as f:
            m = json.load(f)
        if len(m["ad"]["message"]) > 45:
            sorunlar.append("%s: ad 45 karakteri asiyor" % dil)
        if len(m["aciklama"]["message"]) > 132:
            sorunlar.append("%s: aciklama 132 karakteri asiyor (%d)" % (dil, len(m["aciklama"]["message"])))
    if manifest.get("permissions") or manifest.get("host_permissions"):
        sorunlar.append("uzanti izin istiyor - gerekmiyor olmali (her izin magazada gerekce ister)")
    # Uzanti sayfalarinin guvenlik kurali: satir ici betik ve olay ozniteligi yasak,
    # eval / new Function yasak. Biri sizarsa oyun magazada ACILMAZ.
    with open(os.path.join(hedef, "index.html"), encoding="utf-8") as f:
        html = re.sub(r"<!--.*?-->", "", f.read(), flags=re.S)
    for m in re.finditer(r"<script\b([^>]*)>", html, flags=re.I):
        if "src=" not in m.group(1):
            sorunlar.append("index.html icinde satir ici <script> var")
    if re.search(r"\son[a-z]+\s*=", html, flags=re.I):
        sorunlar.append("index.html icinde onclick=... gibi olay ozniteligi var")
    for js in ("index.js", "kabuk.js", "arkaplan.js"):
        with open(os.path.join(hedef, js), encoding="utf-8") as f:
            icerik = f.read()
        if re.search(r"\beval\s*\(|new\s+Function\s*\(", icerik):
            sorunlar.append(js + " eval / new Function iceriyor")
        if re.search(r"https?://", icerik) and js != "index.js":
            sorunlar.append(js + " disaridan bir adrese basvuruyor")
    if sorunlar:
        for x in sorunlar:
            print("  SORUN: " + x)
        hata("Chrome paketi denetimden gecemedi")

    # ---- paket: manifest.json zip'in KOKUNDE olmali ----
    os.makedirs(os.path.join(KOK, "release"), exist_ok=True)
    zyol = os.path.join(KOK, "release", "BorsaPatronu-%s-chrome.zip" % s)
    with zipfile.ZipFile(zyol, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for kok, _, dosyalar in os.walk(hedef):
            for ad in sorted(dosyalar):
                tam = os.path.join(kok, ad)
                z.write(tam, os.path.relpath(tam, hedef).replace(os.sep, "/"))
    with zipfile.ZipFile(zyol) as z:
        adlar = z.namelist()
    if "manifest.json" not in adlar or any(a.endswith(".sav") for a in adlar):
        hata("chrome paketi beklenmeyen icerik: %s" % adlar)
    print("  denetim temiz: MV3, izin yok, satir ici betik yok, eval yok, simgeler %s" % sorted(pngler))
    print("  %s (%.1f MB, %d dosya, surum %s)" % (os.path.relpath(zyol, KOK), os.path.getsize(zyol) / 1e6,
                                                 len(adlar), manifest["version"]))
    return hedef


def magaza_gorselleri():
    """sayfa_gorselleri/chrome/ altina magaza gorsellerini uretir: gercek oyundan
    1280x800 ekran goruntuleri (test/magaza_gorselleri.cpp) + tanitim kutucuklari."""
    env = ortam()
    print("== magaza gorselleri ==")
    os.makedirs(os.path.join(KOK, "sayfa_gorselleri", "chrome"), exist_ok=True)
    exe = os.path.join("test", "magaza_gorselleri.exe")
    komut = ["g++", os.path.join("test", "magaza_gorselleri.cpp"), "-o", exe, "-O2", "-Wno-unused-function"] +         ORTAK_BAYRAKLAR + TEST_BAGLAMA
    s = calistir(komut, env, yakala=True)
    if s.returncode != 0:
        print(s.stderr)
        hata("magaza_gorselleri derlenemedi")
    for dil, klasor in (("tr", "tr"), ("en", "genel")):
        os.makedirs(os.path.join(KOK, "sayfa_gorselleri", "chrome", klasor), exist_ok=True)
        s = calistir([os.path.join(KOK, exe), dil], env, yakala=True)
        print(s.stdout.strip())
        if s.returncode != 0:
            hata("magaza_gorselleri basarisiz: " + dil)
    s = calistir([sys.executable, "magaza_tanitim_uret.py"], yakala=True)
    print(s.stdout.strip())
    if s.returncode != 0:
        print(s.stderr)
        hata("magaza_tanitim_uret.py basarisiz")


def tanitim_videolari():
    """Gercek oynanistan iki tanitim videosu (1920x1080, 30 kare/sn, oyunun muzigi):
    release/video/borsa_patronu_tanitim_tr.mp4 (yerellestirilmis) ve _en.mp4 (genel).
    ffmpeg (libx264) PATH'te olmali."""
    env = ortam()
    ffmpeg = shutil.which("ffmpeg", path=env.get("PATH"))
    if not ffmpeg:
        hata("ffmpeg bulunamadi (winget install Gyan.FFmpeg)")
    print("== tanitim videolari ==")
    exe = os.path.join("test", "tanitim_videosu.exe")
    komut = ["g++", os.path.join("test", "tanitim_videosu.cpp"), "-o", exe, "-O2", "-Wno-unused-function"] +         ORTAK_BAYRAKLAR + TEST_BAGLAMA
    s = calistir(komut, env, yakala=True)
    if s.returncode != 0:
        print(s.stderr)
        hata("tanitim_videosu derlenemedi")
    os.makedirs(os.path.join(KOK, "release", "video"), exist_ok=True)
    for dil in ("tr", "en"):
        cikti = os.path.join(KOK, "release", "video", "borsa_patronu_tanitim_%s.mp4" % dil)
        s = calistir([os.path.join(KOK, exe), ffmpeg, dil, cikti], env, yakala=True)
        print(s.stdout.strip())
        if s.returncode != 0:
            print(s.stderr)
            hata("video uretilemedi: " + dil)


def chrome_sun():
    """Paketi, uzantinin guvenlik kuralini (CSP) taklit eden bir sunucuyla acar:
    tarayicida calisiyorsa, magazadan kurulan uzantida da calisir."""
    import functools
    import http.server
    dizin = os.path.join(KOK, "build", "chrome")
    if not os.path.exists(os.path.join(dizin, "manifest.json")):
        hata("once 'python build.py chrome' calistir")
    with open(os.path.join(dizin, "manifest.json"), encoding="utf-8") as f:
        csp = json.load(f)["content_security_policy"]["extension_pages"]

    class Isleyici(http.server.SimpleHTTPRequestHandler):
        def end_headers(self):
            self.send_header("Content-Security-Policy", csp)
            super().end_headers()

    print("Chrome paketi (uzanti CSP'siyle): http://localhost:8766  (durdurmak icin Ctrl+C)")
    http.server.ThreadingHTTPServer(("127.0.0.1", 8766), functools.partial(Isleyici, directory=dizin)).serve_forever()


def sha256(yol):
    h = hashlib.sha256()
    with open(yol, "rb") as f:
        h.update(f.read())
    return h.hexdigest()


def paketle():
    exe_derle()
    testleri_calistir()
    web = chrome_paketle()
    web = os.path.join(KOK, "web_cikti")
    s = surum()
    os.makedirs(os.path.join(KOK, "release"), exist_ok=True)
    print("== paketler ==")

    # Windows paketi: SADECE .exe + OKUBENI.txt. (Kayit/profil dosyalari, kaynak
    # kod ve eski surumler pakete GIRMEZ.)
    zwin = os.path.join(KOK, "release", "BorsaPatronu-%s-win64.zip" % s)
    with zipfile.ZipFile(zwin, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        z.write(os.path.join(KOK, "borsa_patronu.exe"), "borsa_patronu.exe")
        z.write(os.path.join(KOK, "OKUBENI.txt"), "OKUBENI.txt")
    with zipfile.ZipFile(zwin) as z:
        adlar = z.namelist()
        if sorted(adlar) != ["OKUBENI.txt", "borsa_patronu.exe"]:
            hata("win paketi beklenmeyen dosya iceriyor: %s" % adlar)
        if hashlib.sha256(z.read("borsa_patronu.exe")).hexdigest() != sha256(os.path.join(KOK, "borsa_patronu.exe")):
            hata("paketteki .exe ile klasordeki .exe farkli")
    print("  %s (%.1f MB)" % (os.path.relpath(zwin, KOK), os.path.getsize(zwin) / 1e6))

    # Web paketi: itch.io "HTML" yuklemesi icin index.html kokte olmali.
    zweb = os.path.join(KOK, "release", "BorsaPatronu-%s-web.zip" % s)
    with zipfile.ZipFile(zweb, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for ad in sorted(os.listdir(web)):
            if not ad.startswith("."):
                z.write(os.path.join(web, ad), ad)
    print("  %s (%.1f MB)" % (os.path.relpath(zweb, KOK), os.path.getsize(zweb) / 1e6))


def yayinla():
    """Web surumunu GitHub Pages'e gonderir.

    Yayin dali `gh-pages`dir ve HER SEFERINDE tek commit olarak bastan yazilir
    (zorla gonderim): icinde sadece derlenmis sayfa durur, gecmisi tutulmaz;
    kaynak kodun gecmisi `main` dalindadir. Sayfa adresi:
        https://<kullanici>.github.io/<depo>/
    """
    web = web_derle()
    s = surum()

    def git(*arg, cwd=KOK):
        return subprocess.run(["git"] + list(arg), cwd=cwd, capture_output=True, text=True,
                              encoding="utf-8", errors="replace")

    uzak = git("remote", "get-url", "origin").stdout.strip()
    if not uzak:
        hata("git 'origin' uzak deposu tanimli degil")
    ad = git("config", "user.name").stdout.strip() or "Borsa Patronu"
    eposta = git("config", "user.email").stdout.strip() or "noreply@example.com"

    print("== GitHub Pages ==")
    gecici = tempfile.mkdtemp(prefix="borsa_pages_")
    try:
        for dosya in os.listdir(web):
            shutil.copyfile(os.path.join(web, dosya), os.path.join(gecici, dosya))
        # .nojekyll: GitHub dosyalari oldugu gibi sunsun (Jekyll'den gecirmesin).
        open(os.path.join(gecici, ".nojekyll"), "w").close()
        adimlar = [
            ("init", "-q", "-b", "gh-pages"),
            ("add", "-A"),
            ("-c", "user.name=" + ad, "-c", "user.email=" + eposta, "commit", "-q", "-m", "Web surumu " + s),
            ("push", "-f", uzak, "gh-pages"),
        ]
        for adim in adimlar:
            sonuc = git(*adim, cwd=gecici)
            if sonuc.returncode != 0:
                print(sonuc.stdout + sonuc.stderr)
                hata("git " + adim[0] + " basarisiz")
        print("  gh-pages dalina gonderildi (%s)" % s)
    finally:
        shutil.rmtree(gecici, ignore_errors=True)

    m = re.search(r"github\.com[:/]([^/]+)/([^/.]+)", uzak)
    if m:
        print("  adres: https://%s.github.io/%s/   (yayin 1-2 dakika surebilir)" % (m.group(1).lower(), m.group(2)))


def sun():
    import functools
    import http.server
    dizin = os.path.join(KOK, "web_cikti")
    if not os.path.exists(os.path.join(dizin, "index.html")):
        hata("once 'python build.py web' calistir")
    isleyici = functools.partial(http.server.SimpleHTTPRequestHandler, directory=dizin)
    print("web surumu: http://localhost:8765  (durdurmak icin Ctrl+C)")
    http.server.ThreadingHTTPServer(("127.0.0.1", 8765), isleyici).serve_forever()


def main():
    # Test raporlarinda Turkce harfler var; konsol kod sayfasi ne olursa olsun
    # yazdirirken hata vermesin.
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except (AttributeError, ValueError):
        pass
    hedef = sys.argv[1] if len(sys.argv) > 1 else "exe"
    os.chdir(KOK)
    if hedef in ("-h", "--help", "yardim"):
        print(__doc__)
    elif hedef == "exe":
        exe_derle()
    elif hedef == "test":
        testleri_calistir()
    elif hedef == "web":
        web_derle()
    elif hedef == "chrome":
        chrome_paketle()
    elif hedef == "magaza":
        magaza_gorselleri()
    elif hedef == "video":
        tanitim_videolari()
    elif hedef == "chrome-sun":
        chrome_sun()
    elif hedef == "paket":
        paketle()
    elif hedef == "sun":
        sun()
    elif hedef == "yayinla":
        yayinla()
    else:
        print(__doc__)
        hata("bilinmeyen hedef: " + hedef)


if __name__ == "__main__":
    main()
