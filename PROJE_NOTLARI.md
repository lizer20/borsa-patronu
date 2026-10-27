# Borsa Patronu — Proje Notları

Projenin devir/bağlam dosyası: kod, derleme, oynanış, test ve yayın süreciyle ilgili
bilinmesi gereken her şey burada. Projeye yeni oturan herkes (insan ya da kodlama
asistanı) önce bunu okumalı.

## Ne bu proje

C++17 + raylib ile yazılmış bir borsa yönetim oyunu. Oyuncu 6 hisseyi (ASELS, THYAO,
GARAN, EREGL, KCHOL, BIMAS — gerçek BIST kodları, fiyatlar tamamen simüle) ve iki
güvenli limanı (altın, dolar) alıp satar; haberler, küresel olaylar ve bilançolar
fiyatları oynatır; rütbe atladıkça yeni hisseler ve araçlar (emirler, açığa satış,
güvenli limanlar, kredi) açılır; bir şirketin %50+1'ini toplayan patronu olur ve
yönetim kararları verir. Aradaki boşluğu kısa görevler doldurur.

- **İki dil:** Türkçe ve İngilizce (ilk açılışta sistem diline göre, menüden değişir).
- **Üç dağıtım:** Windows `.exe`, tarayıcı (WebAssembly; GitHub Pages ve itch.io) ve
  **Chrome uzantısı** (Chrome Web Store) — hepsi aynı kaynaktan. Tarayıcı sürümü
  telefonda da oynanır (dikey yerleşim).
- **Her pencereye uyar:** sahne pencerenin en-boy oranına göre boyutlanır; yanlarda
  siyah boşluk kalmaz (bkz. Arayüz kuralları).
- **Asset yok:** ses efektleri, müzik, ikon, kapak görseli ve Türkçe harfler kodla üretilir.
- Değişken/fonksiyon adları ve yorumlar bilerek Türkçe (ASCII Türkçe).

**TASARIM HEDEFİ (kullanıcının sözüyle):** piyasa "en gerçekçi ama aynı zamanda da
oyun olduğunu hissettiren" şekilde olmalı. Piyasa/denge kararlarında ölçü bu.

## Hızlı başlangıç

```
python build.py          Windows .exe'yi derler ve doğrular
python build.py test     denge testi + görsel test (yerleşim denetçisi)
python build.py web      tarayıcı sürümünü derler (web_cikti/)
python build.py chrome   Chrome Web Store uzantı paketi (denetimli) → release/
python build.py magaza   mağaza görselleri (gerçek oyundan ekran görüntüleri)
python build.py video    tanıtım videoları (TR + EN, 1080p) → release/video/
python build.py shorts   YouTube Shorts (dikey, 3 video × TR/EN) → release/video/shorts/
python build.py paket    hepsi + release/ altına zip'ler
python build.py sun      web sürümünü http://localhost:8765 adresinde sunar
python build.py yayinla  web sürümünü derleyip GitHub Pages'e gönderir
```

`build.bat` çift tıklanınca `python build.py` çalıştırır. **Bir şey değiştirdikten
sonra `python build.py test` çalıştır**; hem dengeyi hem arayüz çakışmalarını ölçer.

## Dosya düzeni

Derlenen TEK dosya `borsa_patronu.cpp`'dir ("unity build"): `src/` altındaki
parçaları sırayla içine alır. Yani derleme hâlâ tek komut, ama kod konusuna göre
ayrılmış durumda.

| Dosya | İçerik |
|---|---|
| `src/surum.h` | **Sürüm numarası — tek kaynak** (oyun, `.rc` ve `build.py` buradan okur) |
| `src/temel.h` | **Denge sabitlerinin tamamı**, sahne boyutu kuralları, renk paleti, simülasyonun rastgele sayı üreteci |
| `src/platform.h` | Masaüstü/web farkları: dosya ↔ `localStorage`, sistem dili, tuval boyutu, tık kuyruğu |
| `src/dil.h` | İki dilli metin (`M()`, `IkiDil`), Türkçe harfli font, yazı çizimi, sayı biçimleri |
| `src/ses.h` | Prosedürel ses efektleri ve müzik |
| `src/efekt.h` | Parçacık, toast, buton, ortak çizim yardımcıları |
| `src/veri.h` | **İçerik tabloları:** şirketler, haberler, olaylar, rütbeler, senaryolar, başarımlar, rehber |
| `src/oyun.h` | Oyun durumu (`Hisse`, `Oyun`) — arayüzden bağımsız |
| `src/profil.h` | Başarımlar, rekorlar, günün skoru, tercihler (`borsa_profil.sav`) |
| `src/gorev.h` | Görevler: türler, metinler, ne zaman verilir, nasıl tamamlanır |
| `src/piyasa_islem.h` | Alım/satım ve işlem maliyeti (mikroyapı) |
| `src/piyasa_kaldirac.h` | Bekleyen emirler, kredi, teminat çağrısı, iflas |
| `src/piyasa_olay.h` | Fiyat simülasyonu, haberler, değerleme, bilanço, rütbe ilerlemesi |
| `src/piyasa_sim.h` | Gün döngüsü, temettü, yönetim kararları, rehber, `SimulasyonAdimi()` |
| `src/kayit.h` | Kaydet / yükle (`borsa_kayit.sav`) |
| `src/arayuz_temel.h` | Arayüz veri yapıları ve **yerleşimin tamamı** (`ArayuzKur`: yatay, dikey, pencereler, menüler) |
| `src/arayuz_oyun.h` | Etiketler, üst bar, haber bandı, hisse kartları, adet paneli |
| `src/arayuz_grafik.h` | Ana grafik (üç mod), görev satırı, güvenli liman kartları, rehber, yatay oyun alanının girdisi |
| `src/arayuz_dikey.h` | **Dikey (telefon) oyun ekranı:** üst bar, varlık seçici, tek büyük kart |
| `src/arayuz_pencere*.h` | Pencereler: duraklatma, ayarlar, yönetim, çıkış, emir, kredi, oyun sonu |
| `src/arayuz_menu.h` | Ana menü, senaryo seçimi, başarımlar |
| `src/uygulama.h` | Pencere, sahne boyutunun seçimi, girdi, sabit adımlı kare döngüsü |
| `test/denge_testi.cpp` + `denge_botlari.h` | Penceresiz denge ve suistimal testi |
| `test/gorsel_test.cpp` | Ekran görüntüsü + yerleşim denetçisi + tıklama testi |
| `web/kabuk.html`, `web/kabuk.js` | Web sürümünün sayfa kabuğu ve yardımcı betiği (satır içi betik YOK — bkz. Chrome) |
| `web/site.webmanifest`, `web/sw.js` | Kurulabilir uygulama (PWA): manifest ve internetsiz açılış için service worker |
| `chrome/` | Chrome uzantısının kendi dosyaları: `manifest.json`, `arkaplan.js`, `_locales/` |
| `test/magaza_gorselleri.cpp`, `magaza_tanitim_uret.py` | Mağaza görselleri: gerçek oyundan ekran görüntüleri + tanıtım kutucukları |
| `chrome_magaza_metni.md` | Chrome Web Store sayfa metni, gizlilik cevapları ve yükleme adımları |
| `build.py`, `build.bat` | Derleme betiği |
| `borsa_patronu.rc`, `.manifest`, `.ico` | Windows kaynakları (ikon, sürüm bilgisi, manifest) |
| `ikon_uret.py`, `kapak_uret.py` | İkonu ve itch.io kapak görselini KODLA üreten betikler |
| `OKUBENI.txt` | Dağıtım paketine giren oyuncu README'si (TR + EN) |
| `itch_sayfa_metni.md` | itch.io sayfa metni (TR + EN) |

**Mimari kural:** oyun mantığı (`piyasa_*.h`, `oyun.h`) hiçbir pencere/çizim
fonksiyonu çağırmaz. Bu sayede denge testi aynı kodu pencere açmadan çalıştırır.
Arayüz her kareyi `KareGuncelle()` + `KareCiz()` olarak işler; görsel test bu ikisini
sahte fare girdisiyle sürer.

## Derleme ayrıntıları

Bu makinedeki kurulum (doğrulandı):

- **raylib 6.0:** `C:\raylib\raylib\src` (`libraylib.a` ve web için `libraylib.web.a`).
- **w64devkit:** `C:\Users\Lizer\Downloads\w64devkit\bin` (GCC 16.2) ve
  `C:\raylib\w64devkit\bin` (GCC 15.2). İkisi de PATH'te değil; `build.py` kendisi bulur.
- **Emscripten 5.0.5:** `C:\raylib\emsdk` (raylib kurulumuyla geldi).
- Yollar `build.py`'nin başındaki sabitlerde; başka makinede oraları düzelt.

`build.py` her derlemede şunları **kendiliğinden doğrular** ve aksi halde hata verir:

- Sıfır uyarı (`-Wall`).
- `.exe` GUI alt sisteminde (`-mwindows`; bu bayrak olmadan oyunun arkasında siyah
  bir konsol penceresi açılır).
- `.exe` sadece Windows sistem DLL'lerini istiyor (`-static*` bayrakları sayesinde
  libgcc/libstdc++/winpthread gömülü; yoksa oyun başka bilgisayarda açılmaz).
- Paketteki `.exe` ile klasördeki `.exe` aynı (SHA-256) ve pakette yalnızca
  `borsa_patronu.exe` + `OKUBENI.txt` var.

**Yeni sürümde** yalnızca `src/surum.h` içindeki dört satırı değiştir. Manifestteki
sürüm bilerek sabittir (hiçbir yerde gösterilmez).

## Testler

### Denge testi (`test/denge_testi.cpp`) — pencere açmaz, ~12 sn

1. **Suistimal testleri:** fiyat sabitken al-sat döngüleri (toplu, parçalı, beklemeli,
   açığa satışlı, kredili, altın/dolar üzerinden, 300 rastgele dizi). Hepsi ≤ 0
   olmalı; biri artıya geçerse test başarısız olur.
2. **Piyasa istatistikleri:** hisse başına tik oynaklığı, adil değerden sapma,
   hisseler arası korelasyon, güvenli limanların hisse endeksiyle (eksi) korelasyonu.
3. **Bot turnuvası:** 7 bot × 120 oyun; her rütbeye ulaşma oranı ve medyan gün.
4. **Belirlenim:** aynı tohum birebir aynı oyunu veriyor mu; "Günün Piyasası"nda üç
   farklı bot aynı küresel olayları görüyor mu.
5. **Devralma deneyi:** yönetim kararlarındaki tutumların (hep riskli / hep güvenli /
   şansa bakan / rastgele / hiç devralmayan) sıralaması doğru mu.

**Bir denge sabitini (`src/temel.h`) değiştirdiğinde bu tabloları öncekiyle karşılaştır.**
v2.1 referans değerleri (Klasik, rütbeye ulaşılan medyan gün):

| Bot | R1 | R2 | R3 | R4 | Zirve | Not |
|---|---|---|---|---|---|---|
| Tutucu (al-tut) | 6 | 22 | 55 | 109 | 219 (%48) | hiç satmaz |
| Değerci | 6 | 22 | 57 | 108 | 167 (%68) | al-tut, ama PAHALI/BALON olanı satar, ucuzu alır |
| Haberci | 10 | 25 | 46 | 74 | 107 | haberi %85 isabetle, anında oynar |
| Haberci+K | 9 | 26 | 43 | 61 | 80 | kredi, açığa satış ve güvenli liman da kullanır |
| Rastgele | 11 | 37 | 83 | 164 | 248 (%28) | dikkatsiz oyuncu |
| Çevirici | – | – | – | – | – | sürekli al-sat: 10.000 → ~3.900 TL |
| Pompacı | – | – | – | – | – | parçalı al/toplu sat: 10.000 → ~10.800 TL |

(Bir oyun günü 18 sn; 100 gün = 30 dk.) **v2.0 ile karşılaştırma:** Tutucu 169 (%81),
Haberci 100, Rastgele 244 (%53) idi. Yani ilk üç rütbe (oyunun başı) aynı kaldı,
haberi okuyan oyuncu aynı hızda; değişen tek şey **hiçbir şey yapmadan beklemenin**
artık zirveye taşımaması (bkz. Değerleme). Senaryolar (Haberci): Klasik 110, Kriz
Yılı 199 (%83), Sıfırdan Zirveye 123 gün.

Devralma deneyi (30.000 TL ile en ucuz şirketin 501 hissesi, başka hiçbir şey
yapmadan; 60. gündeki medyan net değer):

| Tutum | Net @60 gün | Zirve (150 günde) |
|---|---|---|
| Şansa bakan (şans ≥ %55 ise riskli) | 97.000 | %69, medyan 90 gün |
| Hep güvenli | 72.000 | %31 |
| Rastgele | 56.000 | %18 |
| Hiç devralmayan (400 hisse tutar) | 51.000 | %4 |
| Hep riskli | 20.000 | %0 |

(v2.0'da "hep riskli" aynı deneyde oyunların **%100'ünde, medyan 29 günde** zirveye
çıkıyordu — bkz. Devralma ve yönetim kararları.)

### Görsel test + yerleşim denetçisi (`test/gorsel_test.cpp`) — ~100 sn

Oyunu gizli pencerede açar, 53 sahneyi **8 sahne boyutunda ve iki dilde** çizer
(848 kare) ve çizilen her yazı/düğme/panelin kutusunu karşılaştırıp şunları raporlar:

- üst üste binen yazılar, bir düğmenin üstüne binen yazı,
- panelinden ya da ekrandan taşan yazı, düğmesine sığmayan etiket.

Boyutlar: yatay 1280×720, geniş 1680×720, en geniş 1920×720, kare 1280×960; dikey
480×900, 480×1000, 480×1180 ve tablet 720×900. Beşinin PNG'si de kaydedilir
(`test/cikti/tr_y_…`, `_g_`, `_k_`, `_d_`, `_t_`); uç boyutlar yalnızca denetlenir.
**Uç boyutlar süs değil:** ilk çalıştırmada üç gerçek hatayı yalnızca onlar yakaladı
(kare pencerede 5 kartın yazıları düğmelere biniyordu, 900 px'lik telefonda senaryo
listesi "Geri" düğmesinin üstüne taşıyordu, tablette bir etiket düğmesine sığmıyordu).

Denetçi önce **kendini sınar** (5 kasıtlı hatanın 5'ini yakalamalı). Ardından sahte
tıklamalarla 67 adımlık bir etkileşim testi koşar: yatayda al/sat, rehber, grafik
modları, duraklatma, ayarlar (dil, renk koru, müzik), emir, açığa satış, güvenli
liman, kredi, teminat çağrısı, görev ödülü, kayıt gidiş-dönüş (V6), eski V5 ve V4
kayıtları, bozuk dosyalar, yönetim kararı; dikeyde varlık seçme, al/sat, emir,
güvenli liman, duraklatma.

**Yeni bir ekran/pencere eklediğinde `Sahneler()` içine bir sahne ekle.**

**Denetçinin göremedikleri (elle bak):** bildirimler (toast) ve farklı katmanlardaki
kutular karşılaştırılmaz. v2.1'de PNG'lere gözle bakınca denetçinin kaçırdığı üç
kusur bulundu: dikeyde duraklatma istatistikleri panelin dışında, karartılmış oyun
ekranının üstünde duruyordu; görev metni "…" ile kırpılıyordu (kırpılan yazı taşmaz,
o yüzden hata sayılmaz); bildirimler rehber kutusunu örtüyordu. **Yeni bir yerleşim
eklediğinde testin temiz çıkması yetmez; PNG'leri aç.**

**Sabit adım tuzağı:** simülasyon 0,05 sn'lik adımlarla ilerler; iki kare (1/30 sn)
bir adım etmez. Testte durumu değiştirdikten sonra düğme konumu okuyacaksan önce
`Kareler(u, 12)` ile en az bir adım geçir (rütbe atlayışı kart yerleşimini değiştirir).

`BORSA_TEST` tanımlıyken `YaziCiz`, `Buton::Ciz` ve `PanelCiz` kutularını kaydeder;
normal derlemede bu kodun hiçbir maliyeti yoktur. Veriye bağlı ya da bilerek taşan
yazılar (kayan bant, kayarak giren bildirim, grafik üstündeki emir harfleri)
`TestKaydiDurdur()/Surdur()` ile muaf tutulur. Modal pencereler `ModalKarart()` ile
yeni bir "katman" başlatır; farklı katmandaki kutular çakışma sayılmaz.

## Oyun sistemleri

Sayıların tamamı `src/temel.h` içinde, gerekçeleriyle birlikte.

### Piyasa modeli

Her saniye, her hisse için:
`r = beyazGürültü·volatilite + beta·PİYASA + ÖZEL − 0,05·(fiyat/adilDeğer − 1)`

- **PİYASA:** tüm hisselerin ortak faktörü, AR(1), yarı ömür ~19,5 sn.
- **ÖZEL (v2.0):** hisseye özgü, yine kalıcı (AR(1)) bir faktör — her hissenin kendi
  trendi. Büyüklüğü betayla artar.
- **Neden eklendi:** v1.x'te iki hissenin 30 saniyelik getirileri arasındaki
  korelasyon **0,98** idi (hepsi aynı çizgiyi çiziyordu, çeşitlendirme anlamsızdı).
  Şimdi 1 sn'de 0,39, 30 sn'de **0,54** (gerçek borsalarda ~0,3–0,5). Hisselerin
  tek tek oynaklığı (tik başı %0,35 BIMAS … %0,63 THYAO) ve adil değerden tipik
  sapması (%6–13) **eskisiyle aynı** tutuldu.
- `RastgeleGauss()`'un std sapması 1 değil **0,289**'dur; `volatilite = 0,008`
  gerçekte tik başı ~%0,23 demektir. |x| ≤ 1 (kuyruklar kesik, tek tikte fiyat uçamaz).
- Oynaklık kümelenmesi: büyük hareket sonrası oynaklık sıçrar, yavaşça söner. Taban =
  baz + hissenin risk primi (riskli yönetim kararları) + küresel risk primi (kötü
  küresel olaylar). İkisi de **kalıcıdır**: oyun ilerledikçe piyasa sertleşir.

### Belirlenim: kendi rastgele sayı üreteci ve sabit adım (v2.1)

- Simülasyon raylib'in `GetRandomValue`'sunu **kullanmaz**; `temel.h` içindeki kendi
  üretecini (`Rng`, xorshift64*) kullanır. İki akış vardır:
  - `g_rngPiyasa` — **ortam:** fiyat gürültüsü, haberler, küresel olaylar, bilançolar.
  - `g_rngOyuncu` — **oyuncuya bağlı** çekilişler: yönetim kararının sonucu ve
    zamanlaması, içeriden bilgi mektubu, görev seçimi.
  Ayrımın sebebi: oyuncunun yaptıkları ortam akışından çekiliş "çalmasın". Ortam
  akışından çekiliş yapan her kod, oyuncunun durumundan **bağımsız sayıda** çekiliş
  yapmak zorundadır (ör. `AgirlikliOlaySec` her zaman 2 kez çeker, bilanço çekilişleri
  koşulsuzdur). Yeni bir rastgele olay eklerken buna dikkat et; denge testinin 4.
  bölümü bozulursa bunu ihlal etmişsindir.
- `SimulasyonAdimi` her zaman `SIM_ADIM_SN` (0,05 sn) ile çağrılır; kare süresi
  biriktirilip eşit adımlarla harcanır (`OyunEkraniGuncelle`). Kare hızı ne olursa
  olsun aynı tohum aynı piyasayı verir.
- Üreteçlerin durumu kayda yazılır: "kaydet – dene – beğenmezsen geri yükle" ile bir
  yönetim kumarının ya da haberin sonucu değiştirilemez.
- raylib'in üreteci yalnızca görsel efektlerde (parçacıklar) kullanılır.

### İşlem maliyeti / mikroyapı (v2.0: akış tabanlı)

Üç sürümde üç model; ilk ikisi suistimal edilebiliyordu:

- **v1.0:** dolum eski fiyattan, etki sonra → al-sat çevirmek tur başı +%6 risksiz kâr.
- **v1.1:** karekök etki + dolum fiyatı → tek seferde al-sat zarar. **Ama** etki
  emir başına hesaplandığı için karekökün içbükeyliği açık bıraktı: 5'er adet hızlı
  alıp toplu satmak tur başı **+%7** veriyordu (gerçek kodla ölçüldü; o sırada
  "varyantların hepsi negatif" diye yanlış rapor edilmişti).
- **v2.0:** etki, hissenin **birikimli net emir akışına** (`Hisse::akis`, Q) bağlı bir
  durum fonksiyonu:
  - `f(Q) = ETKI_OLCEK · işaret(Q) · √(|Q|/1000)` — anlık "baskı".
  - Q₀→Q₁ giden emrin ortalama dolum sapması = `(F(Q₁) − F(Q₀)) / (Q₁ − Q₀)`,
    F = f'nin integrali; üstüne spread'in yarısı.
  - Q her saniye `AKIS_SONME` ile söner (yarı ömür ~6 sn).
  - **Piyasa fiyatı oyuncunun emriyle değişmez**; baskı yalnızca dolumu etkiler.
- **Neden suistimal edilemez:** ödenen prim F(Q)'da birikir. Net adedi sıfır olan
  kapalı bir döngüde toplam nakit akışı = −spread − (sönen baskı) ≤ 0. Emri bölmek
  hiçbir şey değiştirmez; **beklemek ucuzlatır, acele etmek pahalılaştırır** (gerçek
  piyasadaki "emri zamana yay" mantığı).
- Nötr durumdan tek emrin maliyeti v1.1 ile aynı: 1 adet %0,24 … 1000 adet %3,15.
- **Oyuncuya görünür:** adet panelinde `maliyet ~%1,65`, kartta `baskı +%2,1`, haber
  bandında dolum fiyatı ve maliyet yüzdesi. Görünmez vergi gibi hissettirmemeli.
- Ayar: `ALIM_SATIM_SPREAD` çevirme maliyetini, `ETKI_OLCEK` büyük/aceleci emrin
  maliyetini, `AKIS_SONME` baskının süresini belirler.

### Haberler, küresel olaylar, acemi koruması

- Şirket haberi duyurulur, etkisi **5 sn sonra** fiyata yansır; o sırada haber bandı
  sabitlenir ki okunabilsin. Oyunun çekirdek döngüsü budur.
- Küresel olaylar tüm piyasayı betayla orantılı etkiler; vurgulanan sektör ayrıca
  (bazen ters yönde) etkilenir. Kötü olaylar küresel risk primini kalıcı yükseltir.
- **Küresel haberlerde bilerek yeşil/kırmızı kullanılmaz** (turuncu): iyi mi kötü mü
  olduğunu oyuncu okuyup tahmin etmeli. Bu, kullanıcının zorluk için istediği bir şey.
- **Acemi koruması:** olayın iyi mi kötü mü olacağı havuzdaki orana değil rütbeye
  bağlıdır (`AcemiKorumasi()` + `AgirlikliOlaySec()`). İyi haber olasılığı rütbe 0'da
  %85 (şirket) / %75 (küresel), son rütbede %67 / %40. `ACEMI_*_TABAN` değerleri
  oyunun zor sonunu belirler. Bu yüzden `veri.h`'ye haber eklemek dengeyi değil
  yalnızca çeşitliliği değiştirir. "Kriz Yılı" senaryosunda koruma kapalıdır.

### Değerleme (v2.1) — "pahalı hisse risklidir"

**Sorun (ölçüldü):** hiç satmayan, hiçbir beceri göstermeyen bot oyunların %81'inde
zirveye çıkıyordu. İyi haber kötüden sık geldiği için her hisse sonsuza dek
yükseliyor, oyunun sonu beceriyi değil sabrı ölçüyordu.

**Çözüm:** her hissenin gizli bir **esas değeri** var (`Hisse::esasDeger`). Şirket
haberlerinin bir kısmı esasa yansır, kalanı *heyecandır*. `Degerleme = adilDeğer /
esasDeğer`. Hisse pahalılaştıkça o hisseye **iyi haber gelme olasılığı düşer**
(`− 1,5·ln(değerleme)`), eşiği (1,30) aşarsa her gün bir **DÜZELTME** haberi riski
doğar; ucuzlayan hissede tersi (**TOPARLANMA**). Kartta etiket durur:

| Etiket | Değerleme | Anlamı |
|---|---|---|
| KELEPİR | < 0,85 | belirgin artı beklenti, toparlanma olasılığı |
| UCUZ | < 0,97 | artı beklenti |
| MAKUL | ≤ 1,12 | nötr |
| PAHALI | ≤ 1,27 | eksi beklenti |
| BALON | > 1,27 | belirgin eksi + düzeltme riski |

- **Oyunun başıyla uyum:** heyecan payı rütbeyle büyür (rütbe 0'da sıfır, son rütbede
  %50). İlk rütbelerde değerleme hiç oynamaz, etiket hep MAKUL kalır; oyunun başı
  v2.0 ile aynıdır. **Ama etiketin anlamı her rütbede aynıdır** — ilk tasarımda ceza
  da rütbeyle ölçekleniyordu ve "PAHALI görünce sat" kuralı orta rütbelerde zarar
  ettiriyordu (etiket yazıyor ama etkisi yoktu); Değerci botu bunu gösterdi.
- Esas değer her gün adil değerle arasındaki farkın %3'ünü kapatır
  (`DEGERLEME_YAKINSAMA_GUNLUK`): şirket değerlemesinin "içini doldurur". Bu olmadan
  denge noktasında haber getirisi sıfıra iniyor, haberleri oynayan oyuncu da
  yavaşlıyordu. %6 denendi: al-tut yeniden %68'e çıkıyor, beceri farkı daralıyordu.
- Yönetim kararlarının etkisi esas değere **aynen** işlenir (heyecan sayılmaz).
- Güvenli limanların değerlemesi yoktur (hep 1).

### Görevler (v2.1)

Rütbeler arasındaki boşluğu dolduran kısa hedefler; her an **tek** görev vardır ve
grafik panelinin başlığında durur. 12 tür (`src/gorev.h`): tek satışta X TL kâr, iyi
haberi fiyata yansımadan al, K farklı varlıkta pozisyon tut, net değeri X'e çıkar,
temettü al, olumlu bilançoyu hisseyle karşıla, UCUZ/KELEPİR al, PAHALI/BALON'u kârla
sat, bir emrin gerçekleşsin, açığa satıştan kâr et, kötü küresel haberde altın/dolar
tut, bir şirketi devral.

- Görev oyuncunun **o anki durumuna göre** seçilir (yalnızca açık araçlarla ilgili
  olanlar, bir önceki türün tekrarı değil) ve mekanikleri kullanmaya yönlendirir.
- Ödül: net değerin %2'si (en az 150 TL). Süre 8 gün; dolarsa ceza yok, yenisi gelir.
- Yeni tür eklemek: `gorev.h` başındaki dört adım.

### Güvenli limanlar: altın ve dolar (v2.1)

Rütbe 2'de (`LIMAN_MIN_RUTBE`) açılır. Kodda birer `Hisse`'dir (`liman = true`),
hisse dizisinin **sonunda** dururlar (`HisseSayisi()` = 6 gerçek hisse; limanlar
ondan sonra). Hisselerden farkları:

- Piyasa faktörüne **ters** tepki verirler (beta −0,20 / −0,35) ve kötü küresel
  olaylarda **yükselirler** (`kureselDuyarlilik` −0,55 / −0,80). Ölçülen korelasyon:
  altın −0,36, dolar −0,62. Oynaklıkları hisselerin yarısından azdır.
- Uzun vadede yavaşça değer kazanırlar (günde ~%0,1) ama hisselerin haber getirisi
  yoktur: **büyümek için değil korunmak için** vardır.
- Şirket haberi, bilanço, temettü, değerleme, açığa satış ve devralma **yoktur**.
- Likiditeleri farklıdır (`likidite`, `azamiAdet`): 40 TL'lik dolardan 100.000 adet
  alınabilir, 3.000 TL'lik altından 2.000.
- Yatayda grafik panelinin altında mini kart, dikeyde varlık seçicide çip olarak durur.

### Bilanço takvimi (v2.0)

Her şirket 12 günde bir bilanço açıklar. **Tarih bellidir** (kartta geri sayım),
sonuç değildir; kartta bir beklenti ipucu durur (olumlu / belirsiz / olumsuz) ve %70
tutar. Gerçek piyasadaki gibi beklenti fiyatlanmıştır: beklenen sonuç küçük (%5–9),
sürpriz büyük (%8–14, ters yönde) hareket ettirir. Açıklama normal haber gibi 5 sn
gecikmeyle fiyata yansır. Baskı modeliyle birlikte anlamı şu: olaya **sakin sakin
hazırlanmak ucuz, son anda girmek pahalı**.

### Kademeli açılma

Hisseler en ucuzdan başlayarak her rütbede bir tane açılır (kilitli hisseler tamamen
gizlidir). Araçlar da rütbeyle açılır ve duyurulur:

| Rütbe | Açılan |
|---|---|
| 1 Deneyimli Yatırımcı | Emirler (`EMIR_MIN_RUTBE`) |
| 2 Uzman Trader | Açığa satış (`ACIGA_MIN_RUTBE`) + güvenli limanlar (`LIMAN_MIN_RUTBE`) |
| 3 Borsa Kaplanı | Kredi (`KREDI_MIN_RUTBE`) + içeriden bilgi mektubu |

### Emirler

Kartlardaki **EMİR** düğmesi: zarar kes (stop), kâr al (hedef), limit alım. Her
hissede her türden bir tane. Seviyeler güncel fiyata göre hazır yüzdelerdir (en
yakını %3). Emirler **piyasa emri** olarak gerçekleşir: tetik fiyatı garanti değildir
(haberle boşluk yapan fiyatta dolum daha kötü olur — gerçekçi). Pozisyon kapanınca
stop/kâr al kendiliğinden silinir. Aktif emirler kartta etiket, grafikte kesikli
çizgi olarak görünür.

### Açığa satış

Elde hisse yokken **SAT** açığa satar (düğme "AÇIĞA SAT" olur; açıktayken AL "AÇIĞI
KAPAT" olur). Tek işlemde pozisyonun işareti değişmez (yanlışlıkla açığa düşülmez).
`Hisse::adet < 0` açık demektir; `maliyet` her iki yönde pozitif tutulur. Açıklar
günlük ödünç ücreti ve temettü öder. Hisse başına en fazla 500 adet.

### Kredi, teminat çağrısı, iflas

- Yükümlülük = borç + açığa satılan hisselerin güncel değeri.
- Teminat oranı = net değer / yükümlülük.
- Yeni borç/açık: sonrasında yükümlülük ≤ net değer olmalı (2 kat kaldıraç).
- Oran %60 altı: uyarı. **%40 altı: teminat çağrısı** — tüm açıklar kapatılır, borç
  bitene kadar en büyük pozisyondan hisse satılır.
- Günlük faiz %0,3 (bir gün 18 sn); nakit yetmezse anaparaya eklenir.
- Net değer başlangıç sermayesinin %2'sinin altına inerse **iflas** (oyun sonu).
  Rütbe hiç düşmediği için aksi halde oyuncu hiçbir şey alamadan takılı kalırdı.

### Devralma ve yönetim kararları

1000 hissenin 501'i → şirketi devralırsın. Sahipken 14–24 sn'de bir karar penceresi
açılır (6 şablon). Çoğunluğu kaybedersen kontrol gider. Karar açıkken simülasyon
durur, kaçılamaz.

**v2.1: riskli seçenek gerçekten risklidir.** v2.0'a kadar riskli seçenek her
seferinde +%8…18 veriyor, tek bedeli biraz oynaklıktı. Ölçüldü: şirketi devralıp
*hep* riskliyi seçen oyuncu 30.000 TL'den zirveye medyan **29 günde**, oyunların
**%100'ünde** çıkıyordu (haberleri okuyan bot aynı yolu ~80 günde alıyor) — oyunun
geri kalanını anlamsızlaştıran bir açık. Şimdi:

- Riskli karar bir **kumardır**: tutarsa şablondaki kazanç, tutmazsa onun %75'i kadar
  kayıp (`YONETIM_BASARISIZLIK_ORANI`). Sonuç anında bildirim ve haber bandıyla duyurulur.
- **Tutma şansı şirketin birikmiş risk primine bağlıdır:** temiz şirkette %70, risk
  primi tavandayken %30 (`YONETIM_BASARI_TABAN`, `_RISK_EGIMI`). Her riskli karar
  (tutsa da tutmasa da) risk primini artırır; her güvenli karar düşürür
  (`YONETIM_GUVENLI_TOPARLANMA`).
- Doğru oyun: **şirket sağlamken risk al, kırılganken toparla.** Hep riskli batırır,
  hep güvenli az ama kesin kazandırır, şansa bakan en iyisidir (bkz. Testler'deki tablo).
- Pencere bunu **açıkça** gösterir: "Şirket riski: DÜŞÜK/ORTA/YÜKSEK", riskli düğmede
  "Şans %58: +%15 ya da −%11", güvenli düğmede "Kesin +%2, şirket riski azalır".
- Sonuç oyuncu üretecinden çekilir ve üretecin durumu kayda yazılır; kaydedip geri
  yükleyerek sonuç değiştirilemez.

### Diğerleri

- **Temettü:** 5 günde bir portföy değerinin %0,5'i (açıklar için ödenir).
- **İçeriden bilgi mektubu:** rütbe 3+'ta nadiren gelen zarf. Yükseliş, mektup
  **oluştuğu anda** planlanır; okusan da okumasan da gerçekleşir (mektup olayı
  yaratmaz, önceden haber verir). Haber bandına bilerek yazılmaz, kaydedilmez.
- **Senaryolar** (`veri.h`): Klasik, Kriz Yılı (oynaklık tavanda, koruma yok), Zamana
  Karşı (45 gün, skor = net değer), Sıfırdan Zirveye (2.500 TL), Günün Piyasası.
  Her birinin rekoru profilde tutulur. **Sıra değişmemeli** (profil indeksle saklar);
  yenisi sona.
- **Günün Piyasası (v2.1):** 30 günlük, **günün tarihiyle tohumlanan** senaryo. O gün
  oynayan herkes aynı küresel olayları, aynı piyasa havasını ve aynı bilanço
  sonuçlarını görür; skor = 30. gündeki net değer. Günün en iyi skoru profilde
  tutulur, oyun sonu penceresindeki **Skoru Kopyala** düğmesi paylaşılacak metni
  panoya koyar. Ortamın oyuncudan bağımsız kalması için bu senaryoda acemi koruması
  rütbeye değil sabit bir değere bağlıdır (bkz. Belirlenim). Fiyatların son
  basamağı platformlar arasında (Windows / tarayıcı) farklı çıkabilir; olaylar aynıdır.
- **Başarımlar:** 18 tane, profilde kalıcı. Sıra değişmemeli; yenisi sona.
  "Hız Rekoru" eşiği (100 gün) Haberci botunun medyanına yakındır (107).
- **Rehber:** hiç görülmediyse Klasik'te kendiliğinden başlar; 7 adım. Bazı adımlar
  oyuncu isteneni yapınca geçer (al, sat), diğerleri "Anladım" bekler. Ayarlardan
  yeniden başlatılabilir. **Metinlerde "soldaki / sağ alttaki" gibi yön sözcüğü
  kullanma:** yatay ve dikey yerleşimde aynı şey farklı yerde durur; yeri, kutudan
  hedefe uzanan çizgi gösterir.
- **Oyun sonu penceresi:** zirve / süre doldu / iflas; istatistikler ve rekor. Zirve
  ve süre sonrasında oyuncu kum havuzu olarak devam edebilir.
- **Müzik (v2.1):** 24 saniyelik, kendi üstüne sarılan sakin bir döngü (La minör – Fa –
  Do – Sol; ped + bas + arpej + sarmalı yankı), `ses.h` içinde kodla üretilir. Ayarlardan
  kapatılır; "Ses: Kapalı" müziği de susturur. Tarayıcıda ses ilk dokunuşla başlar
  (tarayıcı kuralı).
- **Renk körü modu (v2.1):** Ayarlar'da. Yeşil/kırmızı yerine mavi/turuncu kullanır.
  `RENK_YESIL/KIRMIZI/UYARI` bu yüzden sabit değil değişkendir (`PaletiAyarla`);
  düğme renkleri `ArayuzKur` içinde yeniden atanır. **Bir rengi `static const` bir
  tabloya kopyalama** — palet değişince eski renkte kalır.
- **Grafik modları (v2.1):** TL / yüzde / **net değer geçmişi** (günde bir nokta, bir
  sonraki rütbenin eşiği kesikli çizgiyle). Tercih kayda yazılır ve yeni oyunda korunur.

## Arayüz kuralları

- **Sahne boyutu değişkendir (v2.1).** Her şey sanal bir sahneye çizilir, sonra
  gerçek pencereye basılır; ama sahnenin boyutu artık pencerenin en-boy oranına göre
  seçilir (`SahneBoyutunuGuncelle`, kurallar `temel.h` başında):

  | Pencere oranı (en/boy) | Sahne | Fazla alan nereye gider |
  |---|---|---|
  | 16:9'dan geniş (≤ 8:3) | yükseklik 720, genişlik 1280…1920 | grafiğe |
  | 16:9 … 4:3 | genişlik 1280, yükseklik 720…960 | kartlara ve grafiğe |
  | 0,80'den dar (telefon) | **dikey yerleşim:** genişlik 480…720, yükseklik 900…1180 | grafiğe |

  Bu aralıklarda sahne pencereyi **boşluksuz** doldurur; dışında (çok basık / kareye
  yakın dikey) ortalanır ve kalan yer boş kalır. v2.0'a kadar sahne sabit 1280×720
  idi ve 16:9 olmayan her pencerede (bir tarayıcı sekmesi ~2:1'dir) yanlarda siyah
  bant kalıyordu.
- `PENCERE_GENISLIK` / `PENCERE_YUKSEKLIK` bu yüzden **sabit değil**, o anki sahne
  boyutudur. Yerleşim kodunda `1280` ya da `720` yazma; bunlara göre hesapla.
- **Dikey yerleşim** (`g_dikey`, `arayuz_dikey.h`) ayrı bir ekrandır: 6 kartı alt alta
  dizmek yerine bir **varlık seçici** (her varlık için bir çip) ve **seçili varlığın
  tek büyük kartı** vardır; AL/SAT düğmeleri parmak boyundadır. Üst bar, haber bandı,
  grafik ve adet paneli yatayla aynı fonksiyonlarla çizilir. "Yeni Oyun" düğmesi
  dikeyde ekranda yoktur (yanlış dokunuşla sıfırlama olmasın; ana menüden).
- **Pencereler ve menüler iki yönde aynı koddur;** dar sahnede (`DarEkran()`, genişlik
  < 1000) iç düzenlerini alt alta dizerler. Paneller `OrtaliPanel()` ile sahneye
  sığacak şekilde kısalır.
- **Dikdörtgenler tek yerde** hesaplanır: `ArayuzKur()` (sahne boyutu her değiştiğinde
  yeniden çalışır; düğmelerin hover durumunu bozmaz). Düğme **etiketleri her karede**
  yeniden yazılır (`EtiketleriYenile()`); dil ya da durum değişince eski etiket
  kalması diye bir hata sınıfı olamaz.
- **Grafik başlığında metin kırpılmaz:** görev, ödülü ve içeriden bilgi şeridi tek
  satıra sığmıyorsa alt alta dizilir ve çizim alanı o kadar aşağı iner.
- **Kompakt kart:** 130 px'ten kısa her kart kompakt çizilir ve 95 px'i geçmez
  (denenmiş aralık 59–95). Sahne yüksekse artan yer portföy özetine kalır.
- **Pencere önceliği** (en üstteki tıkı alır): çıkış onayı > oyun sonu > yönetim
  kararı > emir > kredi > ayarlar > duraklatma menüsü. Herhangi biri açıkken
  simülasyon durur.
- **Tık delmesi koruması:** oyun alanının tıklanabilirliği karenin *başındaki* duruma
  göre belirlenir; bu karede kapanan pencerenin düğmesine basılan tık alttaki AL/SAT'a
  geçmez. Duraklatmada "Yeni Oyun" tıklanamaz; ayrıca iki aşamalı onay ister.
- Grafik panelinin **sağ üstü doludur** ("PİYASA MODU" yazısı ve içeriden bilgi
  şeridi) — oraya düğme koyma.
- Bildirimler (toast) her şeyin üstünde çizilir ve duraklatmada da hareket eder.
  Dikeyde rehber kutusu açıksa bildirimler kutunun **altından** başlar.
- Görsel durum (`GorselGuncelle`) simülasyondan ayrıdır ve her karede çalışır.

## Dil ve font

- Her metin çağrı yerinde iki dilde yazılır: `M("Kaydet", "Save")`. Arama tablosu
  yok; eksik çeviri **derleme hatası** olur. Veri tablolarında `IkiDil` kullanılır.
- Kelime sırası dillerde farklıysa açık dal yaz:
  `(g_dil == Dil::EN) ? "Buys " + n + "..." : "... " + n + " alır."`
- Sayılar dile göre: `SayiFormatla`, `Para`, `ParaIsaretli`, `Yuzde`
  (Türkçe `12.345,68` / `%12,4`, İngilizce `12,345.68` / `12.4%`).
- **Font:** raylib'in varsayılan fontu Latin-1'i kapsar (ç, ö, ü, â var) ama
  **ğ Ğ ı İ ş Ş** yoktur. Bu 6 glif `dil.h` içinde, fontun kendi piksel stiline uygun
  olarak **kodla çizilip** atlasa eklenir (`FontHazirla`). Genişlikler taban harflerle
  aynıdır. Oyun metinleri artık **gerçek Türkçe** yazılır (kaynak dosyalar UTF-8).
- Latin-1 + bu 6 harf dışındaki karakterler (—, ★, €, emoji) **hâlâ çizilmez**;
  ikon gerekiyorsa ilkel şekillerle çiz (zarf ve madalya böyle).
- `YaziCiz`/`YaziOlc`, raylib'in `DrawText`/`MeasureText`'iyle birebir aynı ölçü
  kurallarını kullanır (en küçük boyut 10, harf aralığı = boyut/10).

## Kayıt ve profil

- **Oyun kaydı** `borsa_kayit.sav`: satır satır metin. Sürümler V1→V6. **Kural: her
  yeni sürüm alanlarını dosyanın SONUNA ekler**; bu yüzden V2…V5 kayıtları hâlâ
  yüklenir (eksik alanlar varsayılanda kalır, ilk kayıtta V6'ya yükselir). V1 reddedilir.
  Yeni alan eklerken aynı yöntemi kullan: sona ekle, yeni etiket tanımla, eskileri
  kabul edilenler arasında bırak.
- **V6 (v2.1)** şunları ekler: iki rastgele sayı üretecinin durumu, günün tarihi,
  hisselerin esas değerleri, güvenli limanların tamamı (eski blokların ortasına alan
  eklenemeyeceği için limanlar eski bloklarda **yoktur**, kendi bloklarında yazılır),
  görev durumu, net değer geçmişi. Eski bir kayıt yüklenince limanlar başlangıç
  fiyatında, değerlemeler 1,00'da, görev boş başlar. Üreteç durumu yalnızca yükleme
  **başarılıysa** değiştirilir. Kullanıcının gerçek V4 kaydıyla doğrulandı.
- **Profil** `borsa_profil.sav`: başarımlar, senaryo rekorları, dil, "rehber görüldü",
  müzik ve renk körü tercihleri, günün skoru. Oyun kaydından ayrıdır; yeni oyun ya da
  başka kayıt yüklemek bunları silmez. Yeni alanlar burada da sona eklenir.
- Masaüstünde ikisi de **`.exe`'nin yanına** yazılır (çalışma dizinine değil —
  kısayoldan/itch uygulamasından başlatınca kayıt "kayboluyordu"). Webde `localStorage`.
- Yükleyici **istisna kullanmaz** (`SatirOkuyucu`): web derlemesinde istisnalar
  kapalıdır. Bozuk/yarım dosya reddedilir, oyun durumu değişmeden kalır.
- İçeriden bilgi mektubu ve bekleyen küresel olayın sektör vurgusu kaydedilmez.

## Web sürümü

- `python build.py web` → `web_cikti/` (index.html + .js + .wasm, toplam ~0,9 MB).
- Fark yaratan her şey `#if BORSA_WEB` ile `platform.h` ve `uygulama.h` içinde:
  - Kare döngüsünü tarayıcı çağırır (`emscripten_set_main_loop_arg`).
  - Çıkış düğmeleri ve pencere modu seçenekleri gizlidir; tuval sayfayı doldurur.
  - **Tuval boyutu (v2.1):** her karede `js_tuval_hedef_boyutu()` sorulur (sayfanın
    boyutu × ekranın piksel yoğunluğu, yoğunluk en fazla 2) ve tuval farklıysa
    `SetWindowSize` ile uydurulur. Pencere büyüyüp küçülünce ya da telefon dönünce
    sahne ve yerleşim o karede değişir. Yüksek yoğunluklu ekranlarda yazı nettir.
  - **Tık kuyruğu:** raylib fare tuşunu kare başına bir kez yoklar; aynı karede
    basılıp bırakılan tık kaybolur. `mousedown` ayrıca yakalanıp sıraya konur.
  - **Dokunmatik:** raylib dokunuşu fare tuşuna eşler. Tarayıcının dokunuştan sonra
    ürettiği sahte `mousedown` tık kuyruğuna girerse tek dokunuş iki tık olurdu (çift
    alım!); bir dokunuştan sonraki 800 ms içindeki `mousedown` bu yüzden yok sayılır.
- **Test:** tarayıcı görünmeyen bir sayfada `requestAnimationFrame`'i hiç çağırmaz.
  Otomatik testler için adrese `?zamanlayici` eklenirse kareler zamanlayıcıyla üretilir.
- Doğrulandı (yerel sunucu + tarayıcı): 2,4:1 ve 1,43:1 pencerelerde sahnenin
  boşluksuz dolması; telefon öykünmesinde (375×812, yoğunluk 2) dikey yerleşim, net
  yazı, doğru yere düşen ve **tek** sayılan dokunuş; alım, duraklatma, kayıt (V6) ve
  sayfa yenilendikten sonra yükleme; konsolda hata yok. **Gerçek bir telefonda
  denenmedi** (yalnızca tarayıcının telefon öykünmesi); ses de dinlenmedi.

## Yayınlama (GitHub Pages)

Oyunun canlı adresi: **https://emrebiltekin.me/borsa-patronu/**
(`lizer20.github.io/borsa-patronu` da buraya yönlenir; hesabın Pages özel alan adı
`emrebiltekin.me`.)

- `python build.py yayinla` web sürümünü derler ve `gh-pages` dalına gönderir.
  O dal **her seferinde tek commit olarak baştan yazılır** (zorla gönderim): içinde
  yalnızca derlenmiş sayfa durur, geçmişi tutulmaz. Kaynak kodun geçmişi `main`'dedir.
- Pages ayarı: kaynak `gh-pages` dalı, kök klasör. Yayın 1–2 dakika sürer.
- Sayfa tam bir tarayıcı sekmesidir (iframe değil); sekmenin en-boy oranı 16:9
  olmadığı için sahnenin pencereye uyması burada özellikle önemlidir (v2.1 ile
  sahne sekmeyi boşluksuz doldurur; telefonda dikey yerleşime geçer).
- Kayıtlar tarayıcının `localStorage` alanında, alan adına özel tutulur: adres
  değişirse (ör. özel alan adı kaldırılırsa) oyuncuların kayıtları görünmez olur.
- Paylaşım önizlemesi (`og:image`) `web/kabuk.html` içinde TAM adresle yazılıdır;
  alan adı değişirse orayı da güncelle.

### Kurulabilir uygulama (PWA, ChromeOS)

Site bir **kurulabilir web uygulamasıdır**: ChromeOS'ta, masaüstü Chrome/Edge'de ve
Android'de adres çubuğunda "Yükle" simgesi çıkar; kurulunca başlatıcıda/rafta kendi
simgesiyle, kendi penceresinde ve **internetsiz** açılır. ChromeOS'ta bir uzantı
başlatıcıda uygulama olarak görünmez; Chromebook kullanıcısına doğru yol budur.

- `web/site.webmanifest`: ad, renkler, simgeler (192, 512 ve "maskable" 512) ve
  ChromeOS'un zengin kurulum penceresi için 3 ekran görüntüsü (1280×800, `form_factor:
  wide`). Simgeler `build.py` içinde `ikon_uret.py`'nin çizimiyle **kodla** üretilir;
  ekran görüntüleri mağaza görsellerinin İngilizce setinden kopyalanır (`build.py magaza`).
- `web/sw.js`: **önce ağ** stratejisi — internet varsa her dosya taze gelir (yeni sürüm
  yayınlanınca oyuncu eski kodda takılı kalmaz), yoksa son kopya önbellekten verilir.
  `kabuk.js` onu yalnızca http(s) sayfasında ve iframe dışında kaydeder: Chrome
  uzantısında ve itch.io'da çalışmaz.
- Kurulan uygulama sitenin `localStorage`'ını kullanır: **sitedeki kayıtla ortaktır**.
- Chrome uzantısına PWA dosyaları girmez; `build.py chrome` manifest bağlantısını
  uzantının `index.html`'inden siler ve denetler.
- **ChromeOS "App Mall"** Google'ın seçtiği bir vitrindir, doğrudan başvuru formu yok.
  İçerik Google Play'den ve kurulabilir web uygulamalarından gelir; oraya girme şansı
  için yol Play Store'dur: PWA, Bubblewrap ile Trusted Web Activity olarak paketlenip
  Play'e yüklenir (Play geliştirici hesabı, tek seferlik 25 $). Henüz yapılmadı.

## Yayınlama (Chrome Web Store)

**Mağaza sayfası (yayında, 27 Ekim 2026):** https://chromewebstore.google.com/detail/borsa-patronu/pihnfdifhbbbdiickccdkijpnlddojhj
(uzantı kimliği `pihnfdifhbbbdiickccdkijpnlddojhj`; güncellemeler aynı öğeye yüklenir,
yeni öğe AÇILMAZ — açılırsa oyuncuların kaydı eski kimlikte kalır).

Chrome Web Store artık "Chrome uygulaması" kabul etmiyor (2020'den beri kapalı); oyun
bir **uzantı** olarak yayınlanır: araç çubuğundaki simgeye tıklayınca oyun yeni bir
sekmede açılır (`chrome/arkaplan.js`). İlk kurulumda da bir kez açılır.

- `python build.py chrome` web sürümünü derler, `build/chrome/` klasörünü kurar ve
  `release/BorsaPatronu-vX.Y-chrome.zip` üretir. Manifest sürümü `src/surum.h`'den
  yazılır (`v2.1` → `2.1.0`); her yüklemede bir öncekinden büyük olmalı.
- **Manifest V3 kuralları ve neden uyuyoruz:**
  - Uzaktan kod yasak → oyunun tamamı (JS + WASM) pakette.
  - Uzantı sayfalarında **satır içi betik ve `onclick=` gibi öznitelikler yasak** →
    sayfa betiği `web/kabuk.js`'e taşındı (web sitesi de aynı dosyayı kullanır).
  - `eval` / `new Function` yasak → emscripten `-sDYNAMIC_EXECUTION=0` ile derlenir.
  - WebAssembly için CSP'de `'wasm-unsafe-eval'` gerekir (manifestte).
  - **Hiçbir izin istenmez** (sekme açmak izin gerektirmez); her izin mağazada gerekçe
    ve daha uzun inceleme demektir.
- `build.py chrome` bunların hepsini **denetler** ve biri bozulursa paket üretmez:
  manifestin andığı dosyalar, simge boyutları, ad ≤ 45 / açıklama ≤ 132 karakter,
  izin yok, satır içi betik yok, eval yok, betiklerde dış adres yok.
- **Yerel deneme:** `python build.py chrome-sun` paketi uzantının CSP başlığıyla
  `http://localhost:8766` adresinde sunar; orada çalışıyorsa uzantıda da çalışır
  (doğrulandı: CSP ihlali ve konsol hatası yok, oyun başlıyor). Gerçek uzantı olarak
  denemek için Chrome'da `chrome://extensions` → Geliştirici modu → "Paketlenmemiş öğe
  yükle" → `build/chrome`.
- Kayıtlar uzantının kendi `localStorage` alanındadır: web sitesindeki kayıtla
  **ortak değildir**. Uzantı kaldırılırsa kayıt da silinir.
- Simgeler `.exe` ikonuyla aynıdır: `borsa_patronu.ico` içindeki PNG'lerden çıkarılır.
- **Mağaza görselleri** `python build.py magaza` ile üretilir (`sayfa_gorselleri/chrome/`;
  `tr/` yerelleştirilmiş, `genel/` İngilizce — her birinde 5 ekran görüntüsü + küçük tanıtım
  + kayan yazı kutucuğu; simge kökte):
  ekran görüntüleri 1280×800 ve **gerçektir** — `test/magaza_gorselleri.cpp` Klasik
  oyunu baştan başlatıp denge testindeki Haberci botuna oynatır, belirli anlarda sahneyi
  kaydeder (sabit tohum: her seferinde aynı kareler). Tanıtım kutucukları (440×280
  zorunlu, 1400×560 isteğe bağlı) `magaza_tanitim_uret.py` ile kodla çizilir.
- **Tanıtım videoları** `python build.py video` ile üretilir (`release/video/`, depoya
  girmez): `borsa_patronu_tanitim_tr.mp4` (yerelleştirilmiş) ve `_en.mp4` (genel), 51 sn,
  1920×1080, 30 kare/sn, H.264 + AAC. `test/tanitim_videosu.cpp` oyunu gizli pencerede
  **gerçekten oynatır**: imleç gerçek düğmelere tıklar (sahte `Girdi`), uzun bekleyişleri
  Haberci botu oynar ve video bunları karararak atlar; üstüne yalnızca imleç, alt yazı ve
  geçiş çizilir. Ses izi oyunun kendi müziğidir (`MuzikOrnekleri()`). Sabit tohum: iki dil
  birebir aynı oyunu gösterir. ffmpeg (libx264) gerekir; bu makinede winget ile kurulu.
  Üretim sırasında bilgisayardan ses çıkmaz (`sesAcik = false`).
- **YouTube Shorts** `python build.py shorts` ile üretilir (`release/video/shorts/`, depoya
  girmez): 3 video × 2 dil, **dikey 1080×1920**, 12–15 sn — `haber` (oku, al, 5 sn geri
  sayım, kârı al), `devralma` (501. hisse, patronluk, yönetim kumarı), `zirve` (10.000 TL'den
  Efsane Yatırımcı'ya rütbe rütbe). `test/shorts_videosu.cpp` tanıtım videosuyla aynı
  yöntemi kullanır ama oyunu **telefon yerleşiminde** (540×960 sahne, 1,5 kat) çizer;
  üstteki 250 px'lik bant videonun "kancası" olan alt yazıya ayrılmıştır (Shorts'ta alt
  kısım YouTube'un kendi yazılarına kalır). Menü atlanır: ilk saniyeden aksiyon.
- **Gizlilik politikası:** `web/gizlilik.html` → https://emrebiltekin.me/borsa-patronu/gizlilik.html
  (mağaza zorunlu tutuyor). Oyun bir gün veri toplamaya başlarsa (analitik, skor sunucusu)
  bu sayfa ve mağazadaki "Veri kullanımı" formu **aynı anda** güncellenmeli.
- Sayfa metni, gizlilik formunun cevapları ve adım adım yükleme:
  `chrome_magaza_metni.md`. Geliştirici hesabı (tek seferlik 5 $) ve yükleme
  **kullanıcının kendi hesabıyla** yapılır.

## Yayınlama (itch.io)

`python build.py paket` iki dosya üretir:

- `release/BorsaPatronu-vX.Y-win64.zip` — yalnızca `borsa_patronu.exe` + `OKUBENI.txt`.
  itch.io'da "Windows" olarak işaretle.
- `release/BorsaPatronu-vX.Y-web.zip` — itch.io'da "This file will be played in the
  browser" işaretle; görünüm 1280×720, "Fullscreen button" ve "Mobile friendly"
  açık (v2.1 ile oyun her çerçeve boyutuna ve telefona uyar). Tarayıcıda oynanan
  sürümde **SmartScreen uyarısı hiç çıkmaz**.

Dikkat edilecekler:

- **`borsa_kayit.sav` ve `borsa_profil.sav` pakete girmez** (build.py zaten almaz).
- **SmartScreen ("Bilinmeyen yayımcı")** bir virüs tespiti değil, itibar kontrolüdür;
  imzasız ve az indirilmiş her `.exe` alır. Mart 2024'ten beri EV sertifikası bile
  ilk günkü uyarıyı kaldırmıyor; Azure Trusted Signing bireylere yalnızca ABD/Kanada'da
  açık. İtibar dosya **hash**'ine bağlıdır: her yeni `.exe` sıfırdan başlar, o yüzden
  gereksiz sık build yayınlama. Defender gerçek bir tehdit adı verirse (ör.
  Trojan:Win32/Wacatac) yanlış pozitiftir ve Microsoft'a bildirilmelidir.
- Sayfa görselleri `sayfa_gorselleri/` altında; kapak `kapak_uret.py` ile üretilir
  (630×500). Ekran görüntüleri gerçek olmalı: oyunda **F12** (masaüstü) ya da
  `test/cikti/*.png` (görsel testin ürettiği gerçek render'lar).

## Sürümleme

- Uzak depo: **https://github.com/lizer20/borsa-patronu** (herkese açık). Commit'ler
  GitHub'ın `noreply` adresiyle atılır (depo ayarında tanımlı); kişisel e-posta
  depoya girmez.
- Proje bir **git deposu** (v2.0 ile). Her sürüm `vX.Y` etiketiyle işaretlenir
  (`v1.1`, `v2.0`, `v2.1`); fark görmek için `git diff v2.0 v2.1`, geri almak için
  `git checkout`. Yeni sürümün sırası: `src/surum.h` → `python build.py paket` →
  commit + etiket + `git push --tags` → `python build.py yayinla`.
- `updates/v0.1 … v1.1` klasörleri git öncesi elle alınmış kopyalardır; **donduruldu**,
  depoya girmez, yenisi eklenmez. Eski sürümlerin `.exe`'leri orada ve `release/` altında.
- `release/`, `updates/`, derleme çıktıları ve oyuncu dosyaları `.gitignore`'dadır.
- **Commit mesajlarına asistan/araç imzası (Co-Authored-By vb.) ekleme** — kullanıcı
  projede o ismin geçmesini istemiyor.

## Bilinen kısıtlar

- Latin-1 + 6 Türkçe harf dışındaki karakterler çizilmez (bkz. Dil ve font).
- Masaüstünde DPI farkındalığı bilerek kapalı (manifestte yok): pencere yüksek DPI
  ekranlarda Windows tarafından büyütülür. Açılırsa pencere fiziksel olarak küçük
  kalır; önce başlangıç pencere boyutunu DPI'ya göre seçmek gerekir.
- Masaüstü penceresi yeniden boyutlandırılabilir (en az 480×360); çok dar yapılırsa
  dikey yerleşime geçer. Bu bilerek böyle: telefon yerleşimini masaüstünde denemenin
  en kolay yolu.
- Telefon yerleşimi yalnızca tarayıcının telefon öykünmesiyle denendi.
- Ses cihazı yoksa oyun sessiz çalışır (bilerek; `Cal()` ve `SesEfektleri::Yukle`
  içindeki savunmayı silme).
- Şirket sayısı kayıt dosyasına yazılır; şirket eklemek/çıkarmak eski kayıtları bozar.
  Yeni bir güvenli liman eklemek bozmaz (limanlar V6 bloğunda sayılarıyla yazılır),
  ama `AZAMI_LIMAN` (4) ve dikey seçicinin satır sayısı gözden geçirilmelidir.
- Web sürümünde kayıt tarayıcıya ve siteye özeldir; itch.io'da üçüncü taraf depolamayı
  engelleyen tarayıcılarda kayıt tutulmayabilir.

## Kullanıcının istek geçmişi (sıra önemli)

1. Gemini'nin Tkinter prototipini iyileştirme (artık geçersiz; proje C++'a taşındı).
2. Haberlerin fiyatı 5 saniye gecikmeli etkilemesi.
3. C++/raylib'e yeniden yazım + "iş yönetimi oyunu" hissi.
4. Haber bandının en yeni haberi okunacak şekilde sabitlemesi.
5. Ayarlar menüsü (ses + pencere modu).
6. Tam ekranda siyah kalan bölge hatası (ölçekleme).
7. Piyasanın daha gerçekçi ama öngörülemez olması.
8. Devralma / yönetim mekaniği.
9. Devralma penceresi ve rozetin taşmadığının doğrulanması.
10. Kayıt / yükleme.
11. Çıkış ve Ana Menü düğmeleri.
12. Üst bar düğmelerinin Duraklatma Menüsü'nde toplanması.
13. Kademeli hisse açılması (6 hisse = 6 rütbe).
14. Zorluk için küresel/makro olaylar.
15. Küresel haberlerde yeşil/kırmızının kaldırılması (bilerek zorluk).
16. 1280×800 → 1280×720.
17. Kod incelemesi düzeltmeleri (v0.4).
18. "Oyun başı çok zor, iyi haber az" → acemi koruması (v0.6).
19. Duraklatma menüsünde görünür "Devam Et" (v0.7).
20. İçeriden bilgi mektubu (v0.8).
21. "Yapabildiğinin hepsini yap": UI/oynanış paketi (v0.9).
22. "Her şey birbirine girmiş": arayüz çakışmaları (v1.0).
23. "Düzelt, piyasa en gerçekçi ama oyun hissi veren şekilde olsun" → mikroyapı (v1.1).
24. "Geliştirmek için ne yapabiliriz, Unity'ye geçelim mi?" → Unity önerilmedi;
    sunulan listenin tamamı istendi ("yapabildiğinin hepsini yap") → **v2.0**: git,
    derleme betiği, modüler yapı, denge testi, yerleşim denetçisi, web sürümü, gerçek
    Türkçe harfler, İngilizce, rehber, emirler, açığa satış, kredi, bilanço takvimi,
    korelasyon ayarı, senaryolar, başarımlar, oyun sonu. Bu sırada v1.1'deki
    "parçalı al, toplu sat" açığı bulundu ve akış tabanlı modelle kapatıldı.
25. "Her şey çok güzel çalışıyor, başka ne getirebiliriz?" → sunulan 6 maddenin
    tamamı istendi ("yapabildiğinin hepsini yap") **artı** "bilgisayarda internette
    görüntüyü sağda ve solda black bar olmadan yap, daha geniş bir görünüm olur"
    **artı** "GitHub deposuna kur, kendin pushla" → **v2.1**: değerleme katmanı
    (al-tut baskınlığı), görevler, güvenli limanlar, Günün Piyasası, telefon
    desteği, müzik, renk körü modu, net değer grafiği; pencereye uyan sahne.
    Doğrulama sırasında "devral + hep riskli" açığı bulundu ve yönetim kararları
    gerçek bir risk/ödül seçimine çevrildi.
26. "GitHub Pages'ten yayınlayalım" → `build.py yayinla`, canlı adres.
27. "Oyunu Chrome mağazasına koymaya uygun hale getirebilir miyiz" → Manifest V3
    uzantı paketi (`build.py chrome`, denetimli), satır içi betiklerin kaldırılması,
    gerçek oyundan mağaza görselleri (`build.py magaza`), mağaza metni ve yükleme adımları.
28. "Bir yerelleştirilmiş bir de genel tanıtım videosu hazırlar mısın" → gerçek oynanıştan
    Türkçe ve İngilizce tanıtım videoları (`build.py video`).
29. Mağaza formu soruları (tek amaç, uzak kod: Hayır, veri kullanımı: hiçbiri), gizlilik
    politikası sayfası; uzantı incelemeden geçti ve **yayınlandı** (27 Ekim 2026).
30. "ChromeOS'ta 'Install' düğmesi yok, App Mall'da da yok, oraya nasıl ekleriz" → site
    kurulabilir uygulama (PWA) yapıldı: manifest, kodla üretilen simgeler, internetsiz
    açılış (`web/sw.js`). App Mall için yol Play Store (TWA) — kullanıcıya bırakıldı.
31. Reddit tanıtımı (r/playmygame ve r/WebGames'e kullanıcı kendisi paylaştı) ve
    "YouTube için Shorts videoları hazırla" → `build.py shorts`.

## Sonraki adımlar için fikirler (istenmedi)

- v2.0 ve v2.1'in yeni sistemleri botlarla ve otomatik testlerle doğrulandı ama
  **insan elinde uzun süre oynanmadı**. Oyuncu geri bildirimiyle ayarlanması
  gerekenler: görevlerin sıklığı ve ödülü, değerleme etiketinin ne kadar "okunur"
  olduğu, yönetim kumarının hissi (şans %70→%30 eğrisi), müziğin seviyesi/tekrarı,
  dikeyde düğme boyları.
- **Gerçek bir telefonda dene** (Android Chrome + iOS Safari): dokunuş, ses başlangıcı,
  adres çubuğunun görünüp kaybolmasıyla sahnenin yeniden boyutlanması.
- Değerleme katmanında beceri farkı orta düzeyde (Değerci %68, Tutucu %48). Daha
  belirgin olması istenirse `DEGERLEME_HABER_EGIMI` ve etiket eşikleri ayarlanabilir.
- Günün Piyasası için ortak skor tablosu (sunucu gerekir; şu an skor panoya kopyalanıp
  elle paylaşılıyor).
- Android paketi (APK): dikey yerleşim hazır; geriye derleme zinciri ve kayıt yolu kalıyor.

## Bu notların yeri

1. Bu dosya (`PROJE_NOTLARI.md`) proje kökünde, `borsa_patronu.cpp` ile aynı klasörde durur.
2. Projenin konumu: `C:\Users\Lizer\Desktop\borsa_patronu\`.
3. Notların kodlama asistanına her oturumda otomatik okutulması, depoya **girmeyen**
   yerel bir köprü dosyasıyla sağlanır (asistanın proje içindeki yerel ayar klasöründe,
   tek satır: `@../PROJE_NOTLARI.md`). O klasörü silme. Depo herkese açık olduğu için
   klasör `.git/info/exclude` ile yok sayılır ve burada adıyla anılmaz.
