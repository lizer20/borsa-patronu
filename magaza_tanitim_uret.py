# -*- coding: utf-8 -*-
"""Chrome Web Store tanitim kutucuklari - oyunun kendi paletiyle, KODLA uretilir.

    <dil>/kucuk_tanitim_440x280.png        magazanin ZORUNLU istedigi kucuk kutucuk
    <dil>/kayan_yazi_tanitim_1400x560.png  "marquee" (kayan yazi / vitrin) kutucugu
    magaza_simgesi_128x128.png             magaza simgesi (dilden bagimsiz)
<dil>: tr (Turkce sayfa) ve genel (Ingilizce; diger butun diller).

Stil kapak_uret.py ile ayni (koyu zemin, yukselen altin cizgi). Ekran goruntuleri
ise gercek oyundan gelir: test/magaza_gorselleri.cpp (python build.py magaza).
"""
import io, os, random
from PIL import Image, ImageDraw, ImageFont

ARKA_UST = (14, 17, 23); ARKA_ALT = (8, 10, 14)
ALTIN = (255, 203, 0); YESIL = (6, 214, 160); KIRMIZI = (239, 71, 111); MAVI = (17, 138, 178)
F = r"C:\Windows\Fonts" + chr(92)
CIKTI = os.path.join(os.path.dirname(os.path.abspath(__file__)), "sayfa_gorselleri", "chrome")


def kutucuk(W, H, alt_yazi, dosya, klasor):
    img = Image.new("RGB", (W, H), ARKA_UST)
    d = ImageDraw.Draw(img)
    for y in range(H):
        t = y / (H - 1)
        d.line([(0, y), (W, y)], fill=tuple(int(ARKA_UST[i] + (ARKA_ALT[i] - ARKA_UST[i]) * t) for i in range(3)))
    adim = max(30, H // 10)
    for gy in range(0, H, adim): d.line([(0, gy), (W, gy)], fill=(26, 30, 38))
    for gx in range(0, W, adim): d.line([(gx, 0), (gx, H)], fill=(24, 28, 36))

    def seri(tohum, bas, bit, oyn):
        rnd = random.Random(tohum)
        n = W // 5 + 1
        s, pts = 0.0, []
        for i in range(n):
            t = i / (n - 1.0)
            s = s * 0.82 + rnd.uniform(-oyn, oyn)
            pts.append((i * 5, bas + (bit - bas) * t + s))
        return pts

    def glow(pts, renk, k):
        d.line(pts, fill=tuple(int(c * 0.2) for c in renk), width=k * 5, joint="curve")
        d.line(pts, fill=tuple(int(c * 0.45) for c in renk), width=k * 2, joint="curve")
        d.line(pts, fill=renk, width=k, joint="curve")

    o = H / 500.0  # kapak_uret.py'deki 500 px yukseklige gore olcek
    glow(seri(21, H * 0.80, H * 0.86, 5 * o), KIRMIZI, max(1, int(2 * o)))
    glow(seri(7, H * 0.87, H * 0.66, 4.5 * o), MAVI, max(1, int(2 * o)))
    altin = seri(3, H * 0.92, H * 0.52, 4.2 * o)
    dolgu = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    ImageDraw.Draw(dolgu).polygon(altin + [(W, H), (0, H)], fill=(255, 203, 0, 30))
    img = Image.alpha_composite(img.convert("RGBA"), dolgu).convert("RGB")
    d = ImageDraw.Draw(img)
    glow(altin, ALTIN, max(2, int(4 * o)))

    # baslik icin ust karartma
    katman = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    kd = ImageDraw.Draw(katman)
    ust = int(H * 0.62)
    for y in range(ust):
        kd.line([(0, y), (W, y)], fill=(8, 10, 14, int(240 * (1 - y / float(ust)) ** 0.5)))
    img = Image.alpha_composite(img.convert("RGBA"), katman).convert("RGB")
    d = ImageDraw.Draw(img)

    # Baslik kutucugun %86'sini gecmesin (kucuk kutucukta yukseklige gore secilen
    # boy tasiyordu): sigana kadar kucult.
    boy = int(H * 0.17)
    while True:
        baslik = ImageFont.truetype(F + "ariblk.ttf", boy)
        x0, _, x1, _ = d.textbbox((0, 0), "BORSA PATRONU", font=baslik)
        if x1 - x0 <= W * 0.86 or boy <= 10: break
        boy -= 1
    alt = ImageFont.truetype(F + "segoeuib.ttf", max(14, int(H * 0.068)))

    def ortala(metin, font, y, renk, golge):
        x0, y0, x1, y1 = d.textbbox((0, 0), metin, font=font)
        x = (W - (x1 - x0)) // 2 - x0
        if golge: d.text((x + golge, y + golge), metin, font=font, fill=(0, 0, 0))
        d.text((x, y), metin, font=font, fill=renk)

    g = max(2, int(H / 120))
    ortala("BORSA PATRONU", baslik, int(H * 0.13), ALTIN, g)
    cy = int(H * 0.36)
    d.line([(W // 2 - int(W * 0.2), cy), (W // 2 + int(W * 0.2), cy)], fill=ALTIN, width=max(1, g // 2))
    ortala(alt_yazi, alt, int(H * 0.40), (230, 233, 238), 0)

    hedef = os.path.join(CIKTI, klasor)
    os.makedirs(hedef, exist_ok=True)
    yol = os.path.join(hedef, dosya)
    img.save(yol, "PNG", optimize=True)
    print("tanitim:", klasor + "/" + dosya, img.size, os.path.getsize(yol), "bayt")


def magaza_simgesi():
    """Magazanin istedigi 128x128 simge. Chrome'un kilavuzuna gore cizim 96x96'dir
    ve her kenarda 16 px SEFFAF bosluk birakilir (magaza simgeyi kendi zeminine
    koyar; kenara dayanan simge digerlerinden iri ve kaba gorunur). Cizim, .exe
    ve uzanti simgesiyle AYNIDIR: borsa_patronu.ico icindeki 256 px'lik PNG."""
    import struct
    kok = os.path.dirname(os.path.abspath(__file__))
    veri = open(os.path.join(kok, "borsa_patronu.ico"), "rb").read()
    adet = struct.unpack_from("<HHH", veri, 0)[2]
    en_buyuk = None
    for i in range(adet):
        w, _, _, _, _, _, uzunluk, ofs = struct.unpack_from("<BBBBHHII", veri, 6 + 16 * i)
        w = w or 256
        if en_buyuk is None or w > en_buyuk[0]:
            en_buyuk = (w, veri[ofs:ofs + uzunluk])
    kaynak = Image.open(io.BytesIO(en_buyuk[1])).convert("RGBA").resize((96, 96), Image.LANCZOS)
    # koseleri hafifce yuvarla (magazadaki diger simgelerle uyumlu)
    maske = Image.new("L", (96, 96), 0)
    ImageDraw.Draw(maske).rounded_rectangle([0, 0, 95, 95], radius=14, fill=255)
    kaynak.putalpha(maske)
    simge = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    simge.paste(kaynak, (16, 16), kaynak)
    yol = os.path.join(CIKTI, "magaza_simgesi_128x128.png")
    simge.save(yol, "PNG", optimize=True)
    print("simge:", os.path.basename(yol), simge.size, os.path.getsize(yol), "bayt")


magaza_simgesi()
# tr/: yerellestirilmis (Turkce sayfa)   genel/: Ingilizce (diger butun diller)
for klasor, kucuk, buyuk in (
    ("tr", "Borsa Yönetim Oyunu", "Haberi oku, 5 saniyede karar ver, borsa imparatorluğunu kur"),
    ("genel", "Stock Market Tycoon", "Read the news, decide in 5 seconds, build your market empire"),
):
    kutucuk(440, 280, kucuk, "kucuk_tanitim_440x280.png", klasor)
    kutucuk(1400, 560, buyuk, "kayan_yazi_tanitim_1400x560.png", klasor)
