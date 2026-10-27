import io, os, struct, zlib

# Oyunun kendi paleti (borsa_patronu.cpp'deki RENK_* sabitleriyle ayni)
ARKA   = (14, 17, 23)
PANEL  = (22, 26, 34)
ALTIN  = (255, 203, 0)
YESIL  = (6, 214, 160)

def ikon_ciz(N):
    """N x N RGBA piksel dizisi: koyu zemin uzerinde yukselen 4 altin cubuk."""
    px = [[(ARKA[0], ARKA[1], ARKA[2], 255) for _ in range(N)] for _ in range(N)]
    s = N / 64.0  # 64px tasarim izgarasina gore olcek

    def dikdortgen(x0, y0, x1, y1, renk):
        for y in range(max(0, int(y0)), min(N, int(y1))):
            for x in range(max(0, int(x0)), min(N, int(x1))):
                px[y][x] = (renk[0], renk[1], renk[2], 255)

    # Hafif ic panel (kenarlarda koyu cerceve birakir -> kucuk boyutta da net durur)
    dikdortgen(3 * s, 3 * s, 61 * s, 61 * s, PANEL)

    # Yukselen 4 cubuk: son cubuk yesil (kar) - "yukselen piyasa" okunsun diye
    cubuklar = [(10, 40), (21, 32), (32, 22), (43, 12)]  # (x, cubugun ust y'si)
    for i, (cx, cy) in enumerate(cubuklar):
        renk = YESIL if i == len(cubuklar) - 1 else ALTIN
        dikdortgen(cx * s, cy * s, (cx + 9) * s, 52 * s, renk)

    # Alt taban cizgisi
    dikdortgen(8 * s, 52 * s, 56 * s, 55 * s, ALTIN)
    return px

def png_yaz(px, N):
    ham = b"".join(b"\x00" + b"".join(bytes(px[y][x]) for x in range(N)) for y in range(N))
    def chunk(tip, veri):
        c = tip + veri
        return struct.pack(">I", len(veri)) + c + struct.pack(">I", zlib.crc32(c) & 0xFFFFFFFF)
    return (b"\x89PNG\r\n\x1a\n"
            + chunk(b"IHDR", struct.pack(">IIBBBBB", N, N, 8, 6, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(ham, 9))
            + chunk(b"IEND", b""))

def maskeli_ikon_ciz(N):
    """Kurulabilir web uygulamasi (PWA) icin "maskable" simge: isletim sistemi
    simgeyi daireye/yuvarlak kareye kirpar ve yalnizca ortadaki %80'lik daire
    guvenlidir. Ayni cizim %70 boyutta, koyu zeminin ortasina konur."""
    ic = int(N * 0.70)
    kucuk = ikon_ciz(ic)
    px = [[(ARKA[0], ARKA[1], ARKA[2], 255) for _ in range(N)] for _ in range(N)]
    o = (N - ic) // 2
    for y in range(ic):
        px[o + y][o:o + ic] = kucuk[y]
    return px

if __name__ == "__main__":
    boyutlar = [16, 24, 32, 48, 64, 128, 256]
    pngler = [png_yaz(ikon_ciz(n), n) for n in boyutlar]

    # ICO kapsayicisi: basslik + her boyut icin dizin girdisi + PNG verileri
    ofs = 6 + 16 * len(boyutlar)
    out = struct.pack("<HHH", 0, 1, len(boyutlar))
    for n, veri in zip(boyutlar, pngler):
        out += struct.pack("<BBBBHHII", n if n < 256 else 0, n if n < 256 else 0, 0, 0, 1, 32, len(veri), ofs)
        ofs += len(veri)
    out += b"".join(pngler)

    yol = r"C:\Users\Lizer\Desktop\borsa_patronu\borsa_patronu.ico"
    io.open(yol, "wb").write(out)
    print("ikon yazildi:", yol, os.path.getsize(yol), "bayt,", len(boyutlar), "boyut:", boyutlar)
