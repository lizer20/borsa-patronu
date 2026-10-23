<!--
  Chrome Web Store sayfasi ve yukleme formu icin hazir metinler (TR + EN).
  Paket:    python build.py chrome   ->  release/BorsaPatronu-vX.Y-chrome.zip
  Gorseller: python build.py magaza  ->  sayfa_gorselleri/chrome/
-->

# Chrome Web Store'a yükleme

## Bir kerelik hazırlık (senin yapman gerekenler)

1. **Geliştirici hesabı:** https://chrome.google.com/webstore/devconsole adresine Google
   hesabınla gir, geliştirici sözleşmesini kabul et ve **tek seferlik 5 ABD doları**
   kayıt ücretini öde. (Ödeme ve sözleşme senin hesabına ait; bunu senin yerine yapamam.)
2. Hesap ayarlarında **iletişim e-postasını doğrula** (doğrulanmadan yayın yapılamaz).
   Yayıncı adı olarak "Emre Biltekin" ya da bir takma ad yazabilirsin; mağazada görünür.

## Önce kendi Chrome'unda dene (2 dakika)

1. `python build.py chrome` (paket `build/chrome/` klasörüne de açılmış olarak düşer).
2. Chrome'da `chrome://extensions` → sağ üstte **Geliştirici modu**nu aç →
   **Paketlenmemiş öğe yükle** → `build/chrome` klasörünü seç.
3. Araç çubuğundaki yapboz simgesinden Borsa Patronu'yu sabitle, tıkla: oyun yeni
   sekmede açılır. Bir alım yap, kaydet, sekmeyi kapatıp yeniden aç, kaydı yükle.

## Yükleme

**Yeni öğe** → `release/BorsaPatronu-v2.1-chrome.zip` dosyasını yükle. Sonra sekmeler:

### Mağaza girişi (Store listing)

- **Dil:** Türkçe (varsayılan). İngilizce'yi ayrıca ekle (aşağıdaki EN metin).
- **Ad ve kısa açıklama** paketten otomatik gelir (`_locales`).
- **Kategori:** Oyunlar *(Games)*. Uzantı kategorileri zaman zaman değişiyor; "Games"
  yoksa "Eğlence / Entertainment".
- **Mağaza simgesi (128×128):** `sayfa_gorselleri/chrome/magaza_simgesi_128x128.png` —
  Chrome kılavuzuna uygun: çizim 96×96, her kenarda 16 px şeffaf boşluk.
- **Görseller dile göre iki klasörde** (her biri aynı oyunu gösterir):
  - `sayfa_gorselleri/chrome/genel/` → **genel** (İngilizce arayüz): mağazanın varsayılan /
    "tüm diller" görselleri ve İngilizce sayfa.
  - `sayfa_gorselleri/chrome/tr/` → **yerelleştirilmiş** (Türkçe arayüz): Türkçe sayfa.
  Her klasörde:
  - **5 ekran görüntüsü (1280×800):** `1_oyun`, `2_oyun_yuzde`, `3_emirler`, `4_yonetim`,
    `5_menu` — hepsi gerçek oyundan (bot oynar, sabit tohum).
  - **Küçük tanıtım kutucuğu (440×280):** `kucuk_tanitim_440x280.png`
  - **Kayan yazı (marquee) tanıtım kutucuğu (1400×560):** `kayan_yazi_tanitim_1400x560.png`
- **Tanıtım videosu:** mağaza yalnızca **YouTube bağlantısı** kabul eder. `python build.py video`
  → `release/video/` altındaki videoları YouTube'a yükle (görünürlük: "Liste dışı" yeterli),
  bağlantıyı yapıştır: Türkçe sayfaya `borsa_patronu_tanitim_tr.mp4`, İngilizce
  sayfaya (ve diğer tüm diller için genel) `borsa_patronu_tanitim_en.mp4`.
  YouTube'da "Çocuklar için yapılmadı" seç; başlık önerisi:
  "Borsa Patronu - Tanıtım" / "Borsa Patronu - Trailer".
- **Gizlilik politikası URL'si:** https://emrebiltekin.me/borsa-patronu/gizlilik.html
  (`web/gizlilik.html`, TR + EN; `build.py yayinla` ile siteye gider).
- **Resmî URL / ana sayfa:** https://emrebiltekin.me/borsa-patronu/
- **Destek URL'si:** https://github.com/lizer20/borsa-patronu/issues

### Gizlilik (Privacy practices)

- **Tek amaç (single purpose):**
  > Borsa Patronu adlı borsa yönetim oyununu yeni bir sekmede açar ve oynatır.
  > *Opens and runs the Borsa Patronu stock market tycoon game in a new tab.*
- **İzin gerekçeleri:** uzantı **hiçbir izin istemez**; bu bölüm boş kalır.
- **Uzaktan kod (remote code):** **Hayır.** Oyunun tüm kodu (JavaScript + WebAssembly)
  pakettedir; dışarıdan hiçbir kod ya da veri yüklenmez.
- **Veri kullanımı:** hiçbir kutuyu işaretleme ("bu uzantı kullanıcı verisi toplamaz").
  Oyun kaydı ve tercihler yalnızca tarayıcının kendi depolama alanında (localStorage)
  durur, hiçbir sunucuya gönderilmez. Ardından üç beyan kutusunu onayla.
- **Gizlilik politikası URL'si:** veri toplanmadığı için zorunlu değil; istenirse
  README'deki şu cümle yeterli: "Borsa Patronu hiçbir kişisel veri toplamaz; oyun
  kaydı yalnızca kendi cihazınızda saklanır."

### Dağıtım

- **Görünürlük:** Herkese açık (ya da önce "Liste dışı / Unlisted" yapıp bağlantıyla dene).
- **Bölgeler:** tümü. **Fiyat:** ücretsiz.

İnceleme genellikle birkaç gün sürer. İzin istemeyen, uzaktan kod çalıştırmayan küçük
bir uzantı olduğu için hızlı geçmesi beklenir — ama sonucu garanti edilemez.

### Güncelleme

`src/surum.h`'de sürümü artır → `python build.py chrome` → panoda **Paket** →
**Yeni paket yükle**. Manifestteki sürüm (`2.1.0` gibi) her yüklemede bir öncekinden
büyük olmalı; `build.py` bunu `src/surum.h`'den kendisi yazar.

---

## Mağaza açıklaması — Türkçe

**Küçük bir sermayeyle borsa imparatorluğu kur.**

10.000 TL ile başlıyorsun. Fiyatlar sen bakarken akıyor, haberler tek tek düşüyor.
Hedef: 250.000 TL'ye ulaşıp **Efsane Yatırımcı** olmak.

**Haberi okumak için 5 saniyen var.** Bir haber fiyata hemen yansımaz; bantta 5 saniye
durur. İyi mi kötü mü olduğu söylenmez — okuyup karar vermek sana kalmış.

• Rütbe atladıkça yeni hisseler ve araçlar: zarar kes / kâr al emirleri, açığa satış,
  altın ve dolar, kredi
• Değerleme etiketleri: pahalılaşan hisse risklidir, ucuzlayan fırsattır
• Bir şirketin 501 hissesini topla, patronu ol; riskli yönetim kararları gerçek birer kumar
• Görevler, bilanço takvimi, dünyayı sarsan küresel olaylar
• Günün Piyasası: o gün herkes aynı piyasayı oynar — skorunu paylaş
• 5 senaryo, 18 başarım, Türkçe ve İngilizce, renk körü modu
• Çevrimdışı oynanır; kayıt yalnızca kendi tarayıcında durur, hiçbir veri toplanmaz

Şirket kodları gerçek BIST kodlarından esinlenmiştir; fiyatlar, haberler ve olaylar
tamamen kurgusaldır. Bu bir oyundur, yatırım tavsiyesi değildir.

## Store description — English

**Build a stock market empire from a small stake.**

You start with 10,000 TL. Prices move while you watch and news drops one item at a time.
The goal: reach 250,000 TL and become a **Legendary Investor**.

**You have 5 seconds to read the news.** A headline does not hit the price right away;
it sits on the ticker for 5 seconds. Nobody tells you whether it is good or bad — you
read it and decide.

• Rank up to unlock new stocks and tools: stop-loss / take-profit orders, short
  selling, gold and dollars, credit
• Valuation tags: pricey stocks are risky, cheap ones are chances
• Collect 501 shares of a company to become its boss; risky management calls are real gambles
• Missions, an earnings calendar, global events that shake the whole market
• Daily Market: everyone plays the same market that day — share your score
• 5 scenarios, 18 achievements, English and Turkish, colorblind mode
• Plays offline; your save stays in your own browser and no data is collected

Tickers are inspired by real BIST tickers; prices, news and events are entirely
fictional. This is a game, not investment advice.
