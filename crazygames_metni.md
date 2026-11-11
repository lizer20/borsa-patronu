<!--
  CrazyGames gonderimi icin hazir metinler ve adimlar.
  Paket:    python build.py crazygames        ->  release/BorsaPatronu-vX.Y-crazygames.zip
  Gorseller: python build.py crazygames-medya ->  sayfa_gorselleri/crazygames/ + release/video/crazygames/
-->

# CrazyGames'e gönderme

Gönderim **senin hesabınla** yapılır: https://developer.crazygames.com → **Submit a game**.
Hesap ücretsizdir.

## Neler hazır

| Ne | Dosya |
|---|---|
| Oyun paketi (HTML5) | `release/BorsaPatronu-v2.1-crazygames.zip` |
| Kapak 16:9 (1920×1080) | `sayfa_gorselleri/crazygames/kapak_1920x1080.png` |
| Kapak 2:3 (800×1200) | `sayfa_gorselleri/crazygames/kapak_800x1200.png` |
| Kapak 1:1 (800×800) | `sayfa_gorselleri/crazygames/kapak_800x800.png` |
| Önizleme videosu yatay (1920×1080) | `release/video/crazygames/borsa_patronu_onizleme_yatay.mp4` |
| Önizleme videosu dikey 2:3 (1080×1620) | `release/video/crazygames/borsa_patronu_onizleme_dikey.mp4` |

Kapaklarda yalnızca oyunun adı var: CrazyGames kenarlık, "Oyna/Yeni" yazısı, simge ve
mağaza logosu kabul etmiyor. Videolar 18 saniye, **sessiz**, hızlandırma ve siyah geçiş yok;
ilk kareleri yatay/dikey kapakla aynı (kural). Hepsi gerçek oynanış.

## Pakette CrazyGames için neler değişti

Oyunun kendisi (`index.wasm`) web sitesindekiyle **aynı dosya**; değişen sayfa:

- **CrazyGames SDK v3** eklendi ve oyun SDK hazır olunca yükleniyor (`web/crazygames.js`).
- **gameplayStart / gameplayStop:** oyuncu oynarken "oynuyor", ana menüde, duraklatmada,
  ayarlarda, oyun sonunda "oynamıyor" bildirilir. **loadingStart / loadingStop** yüklemeyi sarar.
- **happytime:** her rütbe atlayışında (az ve anlamlı: oyun boyunca 5 kez).
- **muteAudio:** CrazyGames sesi kapatınca oyun susar; oyuncunun kendi ses ayarından önce gelir.
- **Kayıt:** SDK'nin Data modülüne yazılır (oyuncu CrazyGames hesabıyla girerse ilerlemesi
  hesabına eşitlenir). Modül kapalıysa tarayıcı deposuna düşer, kayıt kaybolmaz.
- **Dil:** CrazyGames'in bildirdiği dile göre (Türkçe ise Türkçe, değilse İngilizce).
- **Dış bağlantı yok:** "Skoru Kopyala" metnindeki site adresi CrazyGames'te eklenmez; sayfada
  paylaşım (og:) etiketi, kurulabilir uygulama dosyaları ve gizlilik sayfası yok.
- Oyun içinde tam ekran düğmesi yok (CrazyGames kendi düğmesini koyar), çıkış düğmesi yok.
- Reklam **yok**. İlk aşama ("Basic Launch") zaten reklamsız; reklam istenirse "Full Launch"
  için ayrıca eklenir (bkz. aşağı).

`build.py crazygames` paketi denetler: SDK dışında dış adres yok, PWA/og etiketi yok,
dosya sayısı ve boyut sınırın altında (6 dosya, 0,9 MB; sınır 50 MB, mobil ana sayfa için 20 MB).

## Yerel deneme

`python build.py crazygames-sun` → http://localhost:8767 . SDK burada "local" ortamda çalışır
ve yaptığı her şeyi tarayıcı konsoluna yazar ("Requesting gameplay start (local)" gibi).
Asıl deneme yeri CrazyGames'in **Preview** aracıdır (gönderim sayfasında); oradaki QA aracı
kuralları kendisi denetler.

## Gönderim formu

- **Game name:** Borsa Patronu
- **Upload:** HTML5 → `release/BorsaPatronu-v2.1-crazygames.zip`
- **Orientation / devices:** masaüstü (yatay) ve mobil. Mobilde oyun dikey yerleşime
  kendisi geçer; telefon için **portrait** (dikey) seç.
- **Progress Save:** **işaretle** (yoksa Data modülü kapalı kalır; oyun yine çalışır ama
  kayıt hesaba eşitlenmez).
- **Category:** Business / Idle / Simulation (hangisi varsa; en uygunu "Simulation").
- **Tags:** stock market, trading, business, tycoon, economy, simulation, strategy

### Description (EN)

> **Build a stock market empire from a small stake.**
>
> You start with 10,000 TL. Prices move while you watch and the news drops one headline at a
> time. A headline does not hit the price right away: you have **5 seconds** to read it and
> decide. Nobody tells you whether it is good or bad.
>
> Rank up to unlock new stocks and tools: stop-loss and take-profit orders, short selling,
> gold and dollars, credit. Collect 501 shares of a company to become its boss and make risky
> management calls. Watch the valuation tags: pricey stocks are risky, cheap ones are chances.
> Missions, an earnings calendar and global events keep every run different.
>
> Reach 250,000 TL to become a **Legendary Investor**. 5 scenarios, 18 achievements, a Daily
> Market where everyone plays the same market, colorblind mode. English and Turkish.

### Controls (EN)

> **Mouse / touch:** pick a trade size, then BUY or SELL on a stock card. ORDER sets stop-loss
> and take-profit. **Space or P:** pause. **1–8:** trade size. **M:** sound on/off.

## Full Launch (sonra, istenirse)

Basic Launch en az 7 gün ve 500 oynanış sürer (500'e ulaşmazsa 21 günde biter); veriler iyiyse
CrazyGames "Full Launch" önerir. O aşamada gelir için reklam gerekir:

- **Midgame reklam** yalnızca doğal bir arada: oyun sonu penceresi kapanınca ya da yeni oyuna
  geçerken. Oyun sırasında, menü/ayar düğmelerinde **yasak**; iki reklam arası en az 3 dakika.
- **Ödüllü reklam** (isteğe bağlı): ör. "reklam izle, görev ödülünü ikiye katla". Reklam
  hata verirse ödül verilmez; düğme oyunun aktif ekranında durmaz.
- Reklam başlayınca ses kapanır, oyun durur; bitince geri gelir.
- Reklam engelleyicisi olan oyuncu yine oynayabilmeli.
