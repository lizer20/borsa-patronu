# -*- coding: utf-8 -*-
"""itch.io kapak gorseli - oyunun kendi paletiyle, 630x500 (315:250 orani)."""
import random, os
from PIL import Image, ImageDraw, ImageFont

W, H = 630, 500
ARKA_UST = (14, 17, 23); ARKA_ALT = (8, 10, 14)
PANEL    = (22, 26, 34); KENARLIK = (45, 52, 64)
ALTIN    = (255, 203, 0); YESIL = (6, 214, 160); KIRMIZI = (239, 71, 111)
MAVI     = (17, 138, 178); SOLUK = (138, 146, 163)
F = r"C:\Windows\Fonts" + chr(92)
BAR = 58                      # alt bilgi seridi yuksekligi
TABAN = H - BAR               # grafik alaninin alt siniri

img = Image.new("RGB", (W, H), ARKA_UST)
d = ImageDraw.Draw(img)
for y in range(H):
    t = y / (H - 1)
    d.line([(0, y), (W, y)], fill=tuple(int(ARKA_UST[i] + (ARKA_ALT[i] - ARKA_UST[i]) * t) for i in range(3)))
for gy in range(0, H, 50): d.line([(0, gy), (W, gy)], fill=(26, 30, 38))
for gx in range(0, W, 45):  d.line([(gx, 0), (gx, H)], fill=(24, 28, 36))

def seri(tohum, bas_y, bit_y, oynaklik):
    """bas_y'den bit_y'ye giden, gurultulu bir fiyat serisi."""
    rnd = random.Random(tohum)
    n = W // 5 + 1
    sapma, pts = 0.0, []
    for i in range(n):
        t = i / (n - 1.0)
        sapma = sapma * 0.82 + rnd.uniform(-oynaklik, oynaklik)
        pts.append((i * 5, bas_y + (bit_y - bas_y) * t + sapma))
    return pts

def glow(pts, renk, kalin):
    d.line(pts, fill=tuple(int(c * 0.20) for c in renk), width=kalin * 5, joint="curve")
    d.line(pts, fill=tuple(int(c * 0.45) for c in renk), width=kalin * 2, joint="curve")
    d.line(pts, fill=renk, width=kalin, joint="curve")

# Ikincil hisseler (soluk, arka planda) - piyasanin geri kalani
glow(seri(21, 400, 430, 5.0), KIRMIZI, 2)
glow(seri(7,  435, 330, 4.5), MAVI,    2)

# ANA HISSE: altin, sol alttan sag ust'e net bir yukselis (markanin kendisi)
altin_pts = seri(3, 452, 268, 4.2)

# cizginin altini hafif altin dolguyla doldur (derinlik)
dolgu = Image.new("RGBA", (W, H), (0, 0, 0, 0))
ImageDraw.Draw(dolgu).polygon(altin_pts + [(W, TABAN), (0, TABAN)], fill=(255, 203, 0, 34))
img = Image.alpha_composite(img.convert("RGBA"), dolgu).convert("RGB")
d = ImageDraw.Draw(img)
glow(altin_pts, ALTIN, 4)
# son noktada parlayan bir isaret
ex, ey = altin_pts[-1]
ex = min(ex, W - 17)   # sag kenarda kirpilmasin
d.ellipse([ex - 13, ey - 13, ex + 13, ey + 13], fill=(60, 50, 10))
d.ellipse([ex - 6, ey - 6, ex + 6, ey + 6], fill=ALTIN)

# Baslik icin ust karartma (grafik gorunsun ama yazi net okunsun)
katman = Image.new("RGBA", (W, H), (0, 0, 0, 0))
kd = ImageDraw.Draw(katman)
for y in range(250):
    kd.line([(0, y), (W, y)], fill=(8, 10, 14, int(238 * (1 - y / 250.0) ** 0.55)))
img = Image.alpha_composite(img.convert("RGBA"), katman).convert("RGB")
d = ImageDraw.Draw(img)

baslik_font = ImageFont.truetype(F + "ariblk.ttf", 64)
alt_font    = ImageFont.truetype(F + "segoeuib.ttf", 21)
etiket_font = ImageFont.truetype(F + "segoeuib.ttf", 15)
rozet_font  = ImageFont.truetype(F + "seguibl.ttf", 14)

def ortala(metin, font, y, renk, golge=True):
    x0, y0, x1, y1 = d.textbbox((0, 0), metin, font=font)
    x = (W - (x1 - x0)) // 2 - x0
    if golge: d.text((x + 3, y + 4), metin, font=font, fill=(0, 0, 0))
    d.text((x, y), metin, font=font, fill=renk)

ortala("BORSA", baslik_font, 46, ALTIN)
ortala("PATRONU", baslik_font, 112, ALTIN)
d.line([(W // 2 - 130, 194), (W // 2 + 130, 194)], fill=ALTIN, width=2)
ortala("Borsa Yönetim Simülasyonu", alt_font, 206, (230, 233, 238))

d.rectangle([0, TABAN, W, H], fill=PANEL)
d.line([(0, TABAN), (W, TABAN)], fill=KENARLIK, width=2)
ortala("6 HİSSE  ·  CANLI PİYASA  ·  ŞİRKET DEVRALMA  ·  TÜRKÇE", etiket_font, TABAN + 20, SOLUK, golge=False)

rozet = "▲ +%18,4"
x0, y0, x1, y1 = d.textbbox((0, 0), rozet, font=rozet_font)
rw, rh = x1 - x0, y1 - y0
d.rounded_rectangle([W - rw - 44, 18, W - 18, 18 + rh + 15], radius=7, fill=YESIL)
d.text((W - rw - 31, 25), rozet, font=rozet_font, fill=(8, 10, 14))

yol = r"C:\Users\Lizer\Desktop\borsa_patronu\sayfa_gorselleri\kapak_630x500.png"
img.save(yol, "PNG", optimize=True)
print("kapak:", img.size, os.path.getsize(yol), "bayt")
