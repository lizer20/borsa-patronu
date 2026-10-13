#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Borsa Patronu - derleme betigi (tek komutla her sey).

    python build.py           Windows .exe'yi derler ve dogrular
    python build.py test      denge testi + gorsel test (yerlesim denetcisi)
    python build.py web       tarayici surumunu derler (web_cikti/)
    python build.py paket     hepsini yapar, release/ altina zip'leri koyar
    python build.py sun       web surumunu http://localhost:8765 adresinde sunar

Surum numarasi TEK yerden okunur: src/surum.h
Arac yollari asagidaki sabitlerde; baska bir makinede buralari duzelt.
"""
import hashlib
import os
import re
import shutil
import subprocess
import sys
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
             "--shell-file", "web/kabuk.html"]
    s = calistir(komut, env, yakala=True)
    cikti = (s.stdout + s.stderr).strip()
    if cikti:
        print(cikti[-3000:])
    if s.returncode != 0:
        hata("web derlemesi basarisiz")
    dosyalar = sorted(os.listdir(cikti_dizin))
    toplam = sum(os.path.getsize(os.path.join(cikti_dizin, d)) for d in dosyalar) / 1e6
    print("  tamam: web_cikti/ (%s) toplam %.1f MB" % (", ".join(dosyalar), toplam))
    return cikti_dizin


def sha256(yol):
    h = hashlib.sha256()
    with open(yol, "rb") as f:
        h.update(f.read())
    return h.hexdigest()


def paketle():
    exe_derle()
    testleri_calistir()
    web = web_derle()
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
            z.write(os.path.join(web, ad), ad)
    print("  %s (%.1f MB)" % (os.path.relpath(zweb, KOK), os.path.getsize(zweb) / 1e6))


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
    elif hedef == "paket":
        paketle()
    elif hedef == "sun":
        sun()
    else:
        print(__doc__)
        hata("bilinmeyen hedef: " + hedef)


if __name__ == "__main__":
    main()
