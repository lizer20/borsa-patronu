# Borsa Patronu — Proje Notları

Projenin devir/bağlam dosyası: kod, derleme, oynanış, test ve yayın süreciyle ilgili
bilinmesi gereken her şey burada. Projeye yeni oturan herkes (insan ya da kodlama
asistanı) önce bunu okumalı.

## Ne bu proje

C++17 + raylib ile yazılmış bir borsa yönetim oyunu. Oyuncu 6 hisseyi (ASELS, THYAO,
GARAN, EREGL, KCHOL, BIMAS — gerçek BIST kodları, fiyatlar tamamen simüle) alıp
satar; haberler, küresel olaylar ve bilançolar fiyatları oynatır; rütbe atladıkça
yeni hisseler ve araçlar (emirler, açığa satış, kredi) açılır; bir şirketin %50+1'ini
toplayan patronu olur ve yönetim kararları verir.

- **İki dil:** Türkçe ve İngilizce (ilk açılışta sistem diline göre, menüden değişir).
- **İki platform:** Windows `.exe` ve tarayıcı (WebAssembly) — aynı kaynaktan.
- **Asset yok:** ses efektleri, ikon, kapak görseli ve Türkçe harfler kodla üretilir.
- Değişken/fonksiyon adları ve yorumlar bilerek Türkçe (ASCII Türkçe).

**TASARIM HEDEFİ (kullanıcının sözüyle):** piyasa "en gerçekçi ama aynı zamanda da
oyun olduğunu hissettiren" şekilde olmalı. Piyasa/denge kararlarında ölçü bu.

## Hızlı başlangıç

```
python build.py          Windows .exe'yi derler ve doğrular
python build.py test     denge testi + görsel test (yerleşim denetçisi)
python build.py web      tarayıcı sürümünü derler (web_cikti/)
python build.py paket    hepsi + release/ altına zip'ler
python build.py sun      web sürümünü http://localhost:8765 adresinde sunar
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
| `src/temel.h` | **Denge sabitlerinin tamamı**, renk paleti, matematik yardımcıları |
| `src/platform.h` | Masaüstü/web farkları: dosya ↔ `localStorage`, sistem dili |
| `src/dil.h` | İki dilli metin (`M()`, `IkiDil`), Türkçe harfli font, yazı çizimi, sayı biçimleri |
| `src/ses.h` | Prosedürel ses efektleri |
| `src/efekt.h` | Parçacık, toast, buton, ortak çizim yardımcıları |
| `src/veri.h` | **İçerik tabloları:** şirketler, haberler, olaylar, rütbeler, senaryolar, başarımlar, rehber |
| `src/oyun.h` | Oyun durumu (`Hisse`, `Oyun`) — arayüzden bağımsız |
| `src/profil.h` | Başarımlar, rekorlar, dil tercihi (`borsa_profil.sav`) |
| `src/piyasa_islem.h` | Alım/satım ve işlem maliyeti (mikroyapı) |
| `src/piyasa_kaldirac.h` | Bekleyen emirler, kredi, teminat çağrısı, iflas |
| `src/piyasa_olay.h` | Fiyat simülasyonu, haberler, bilanço, rütbe ilerlemesi |
| `src/piyasa_sim.h` | Gün döngüsü, temettü, rehber, `SimulasyonAdimi()` |
| `src/kayit.h` | Kaydet / yükle (`borsa_kayit.sav`) |
| `src/arayuz_temel.h` | Arayüz veri yapıları ve **sabit yerleşim** (`ArayuzKur`) |
| `src/arayuz_oyun.h` | Etiketler, üst bar, haber bandı, hisse kartları, adet paneli |
| `src/arayuz_grafik.h` | Ana grafik, rehber katmanı, oyun alanının girdisi |
| `src/arayuz_pencere*.h` | Pencereler: duraklatma, ayarlar, yönetim, çıkış, emir, kredi, oyun sonu |
| `src/arayuz_menu.h` | Ana menü, senaryo seçimi, başarımlar |
| `src/uygulama.h` | Pencere, girdi ve kare döngüsü |
| `test/denge_testi.cpp` + `denge_botlari.h` | Penceresiz denge ve suistimal testi |
| `test/gorsel_test.cpp` | Ekran görüntüsü + yerleşim denetçisi + tıklama testi |
| `web/kabuk.html` | Web sürümünün sayfa kabuğu |
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

### Denge testi (`test/denge_testi.cpp`) — pencere açmaz, ~8 sn

1. **Suistimal testleri:** fiyat sabitken al-sat döngüleri (toplu, parçalı, beklemeli,
   açığa satışlı, kredili, 300 rastgele dizi). Hepsi ≤ 0 olmalı; biri artıya geçerse
   test başarısız olur.
2. **Piyasa istatistikleri:** hisse başına tik oynaklığı, adil değerden sapma,
   hisseler arası korelasyon.
3. **Bot turnuvası:** 6 bot × 120 oyun; her rütbeye ulaşma oranı ve medyan gün.

**Bir denge sabitini (`src/temel.h`) değiştirdiğinde bu tabloyu öncekiyle karşılaştır.**
v2.0 referans değerleri (Klasik, rütbeye ulaşılan medyan gün):

| Bot | R1 | R2 | R3 | R4 | Zirve | Not |
|---|---|---|---|---|---|---|
| Tutucu (al-tut) | 6 | 20 | 51 | 96 | 169 (%81) | hiç satmaz |
| Haberci | 10 | 27 | 45 | 71 | 100 | haberi %85 isabetle, anında oynar |
| Haberci+K | 10 | 27 | 46 | 61 | 84 | kredi ve açığa satış da kullanır |
| Rastgele | 12 | 36 | 79 | 138 | 244 (%53) | dikkatsiz oyuncu |
| Çevirici | – | – | – | – | – | sürekli al-sat: 10.000 → ~3.400 TL |
| Pompacı | – | – | – | – | – | parçalı al/toplu sat: 10.000 → ~10.300 TL |

(Bir oyun günü 18 sn; 100 gün = 30 dk.) Aynı botlar v1.1 kodunda: Tutucu 158,
Haberci 94, Rastgele 230 — yani **v2.0 zorluğu v1.1 ile aynı**. Tek fark Pompacı:
v1.1'de oyunların %80'inde zirveye çıkıyordu (açık), v2.0'da yerinde sayıyor.
Senaryolar (Haberci): Klasik 110, Kriz Yılı 182, Sıfırdan Zirveye 126 gün.

### Görsel test + yerleşim denetçisi (`test/gorsel_test.cpp`)

Oyunu gizli pencerede açar, 34 sahneyi **iki dilde** çizer (`test/cikti/*.png`) ve
çizilen her yazı/düğme/panelin kutusunu karşılaştırıp şunları raporlar:

- üst üste binen yazılar, bir düğmenin üstüne binen yazı,
- panelinden ya da ekrandan taşan yazı, düğmesine sığmayan etiket.

Denetçi önce **kendini sınar** (5 kasıtlı hatanın 5'ini yakalamalı). Ardından sahte
tıklamalarla 40 adımlık bir etkileşim testi koşar (al/sat, duraklatma, emir, açığa
satış, kredi, teminat çağrısı, kayıt gidiş-dönüş, eski V4 kaydı, bozuk dosya).

**Yeni bir ekran/pencere eklediğinde `Sahneler()` içine bir sahne ekle.** v1.0'da
elle bulunan üç çakışmanın üçünü de bu denetçi kendiliğinden yakalar.

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
| 2 Uzman Trader | Açığa satış (`ACIGA_MIN_RUTBE`) |
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

1000 hissenin 501'i → şirketi devralırsın. Sahipken 14–24 sn'de bir karar penceresi:
riskli seçenek (anında kazanç, kalıcı oynaklık) ya da güvenli. 6 karar şablonu.
Çoğunluğu kaybedersen kontrol gider. Karar açıkken simülasyon durur, kaçılamaz.

### Diğerleri

- **Temettü:** 5 günde bir portföy değerinin %0,5'i (açıklar için ödenir).
- **İçeriden bilgi mektubu:** rütbe 3+'ta nadiren gelen zarf. Yükseliş, mektup
  **oluştuğu anda** planlanır; okusan da okumasan da gerçekleşir (mektup olayı
  yaratmaz, önceden haber verir). Haber bandına bilerek yazılmaz, kaydedilmez.
- **Senaryolar** (`veri.h`): Klasik, Kriz Yılı (oynaklık tavanda, koruma yok), Zamana
  Karşı (45 gün, skor = net değer), Sıfırdan Zirveye (2.500 TL). Her birinin rekoru
  profilde tutulur. **Sıra değişmemeli** (profil indeksle saklar); yenisi sona.
- **Başarımlar:** 15 tane, profilde kalıcı. Sıra değişmemeli; yenisi sona.
  "Hız Rekoru" eşiği (100 gün) Haberci botunun medyanıdır.
- **Rehber:** hiç görülmediyse Klasik'te kendiliğinden başlar; 6 adım. Bazı adımlar
  oyuncu isteneni yapınca geçer (al, sat), diğerleri "Anladım" bekler. Ayarlardan
  yeniden başlatılabilir.
- **Oyun sonu penceresi:** zirve / süre doldu / iflas; istatistikler ve rekor. Zirve
  ve süre sonrasında oyuncu kum havuzu olarak devam edebilir.

## Arayüz kuralları

- Her şey sabit **1280×720** sanal sahneye çizilir, sonra gerçek pencereye en-boy
  oranı korunarak basılır (16:9 monitörlerde ölçek tam sayı çıkar, pillarbox kalmaz).
- **Dikdörtgenler tek yerde** hesaplanır: `ArayuzKur()`. Düğme **etiketleri her
  karede** yeniden yazılır (`EtiketleriYenile()`); dil ya da durum değişince eski
  etiket kalması diye bir hata sınıfı olamaz.
- **Pencere önceliği** (en üstteki tıkı alır): çıkış onayı > oyun sonu > yönetim
  kararı > emir > kredi > ayarlar > duraklatma menüsü. Herhangi biri açıkken
  simülasyon durur.
- **Tık delmesi koruması:** oyun alanının tıklanabilirliği karenin *başındaki* duruma
  göre belirlenir; bu karede kapanan pencerenin düğmesine basılan tık alttaki AL/SAT'a
  geçmez. Duraklatmada "Yeni Oyun" tıklanamaz; ayrıca iki aşamalı onay ister.
- Grafik panelinin **sağ üstü doludur** ("PİYASA MODU" yazısı ve içeriden bilgi
  şeridi) — oraya düğme koyma.
- Bildirimler (toast) her şeyin üstünde çizilir ve duraklatmada da hareket eder.
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

- **Oyun kaydı** `borsa_kayit.sav`: satır satır metin. Sürümler V1→V5. **Kural: her
  yeni sürüm alanlarını dosyanın SONUNA ekler**; bu yüzden V2/V3/V4 kayıtları hâlâ
  yüklenir (eksik alanlar varsayılanda kalır, ilk kayıtta V5'e yükselir). V1 reddedilir.
  Yeni alan eklerken aynı yöntemi kullan: sona ekle, yeni etiket tanımla, eskileri
  kabul edilenler arasında bırak.
- **Profil** `borsa_profil.sav`: başarımlar, senaryo rekorları, dil, "rehber görüldü".
  Oyun kaydından ayrıdır; yeni oyun ya da başka kayıt yüklemek bunları silmez.
- Masaüstünde ikisi de **`.exe`'nin yanına** yazılır (çalışma dizinine değil —
  kısayoldan/itch uygulamasından başlatınca kayıt "kayboluyordu"). Webde `localStorage`.
- Yükleyici **istisna kullanmaz** (`SatirOkuyucu`): web derlemesinde istisnalar
  kapalıdır. Bozuk/yarım dosya reddedilir, oyun durumu değişmeden kalır.
- İçeriden bilgi mektubu ve bekleyen küresel olayın sektör vurgusu kaydedilmez.

## Web sürümü

- `python build.py web` → `web_cikti/` (index.html + .js + .wasm, toplam ~0,8 MB).
- Fark yaratan her şey `#if BORSA_WEB` ile `platform.h` ve `uygulama.h` içinde:
  - Kare döngüsünü tarayıcı çağırır (`emscripten_set_main_loop_arg`).
  - Çıkış düğmeleri ve pencere modu seçenekleri gizlidir; tuval sayfayı doldurur.
  - **Tık kuyruğu:** raylib fare tuşunu kare başına bir kez yoklar; aynı karede
    basılıp bırakılan tık kaybolur. `mousedown` ayrıca yakalanıp sıraya konur.
- **Test:** tarayıcı görünmeyen bir sayfada `requestAnimationFrame`'i hiç çağırmaz.
  Otomatik testler için adrese `?zamanlayici` eklenirse kareler zamanlayıcıyla üretilir.
- Doğrulandı (yerel sunucu + tarayıcı): menü, Türkçe glifler, alım, duraklatma, kayıt
  ve sayfa yenilendikten sonra yükleme.

## Yayınlama (itch.io)

`python build.py paket` iki dosya üretir:

- `release/BorsaPatronu-vX.Y-win64.zip` — yalnızca `borsa_patronu.exe` + `OKUBENI.txt`.
  itch.io'da "Windows" olarak işaretle.
- `release/BorsaPatronu-vX.Y-web.zip` — itch.io'da "This file will be played in the
  browser" işaretle; görünüm 1280×720, "Fullscreen button" açık. Tarayıcıda oynanan
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

- Proje bir **git deposu** (v2.0 ile). Her sürüm `vX.Y` etiketiyle işaretlenir;
  fark görmek için `git diff v1.1 v2.0`, geri almak için `git checkout`.
- `updates/v0.1 … v1.1` klasörleri git öncesi elle alınmış kopyalardır; **donduruldu**,
  depoya girmez, yenisi eklenmez. Eski sürümlerin `.exe`'leri orada ve `release/` altında.
- `release/`, `updates/`, derleme çıktıları ve oyuncu dosyaları `.gitignore`'dadır.
- **Commit mesajlarına asistan/araç imzası (Co-Authored-By vb.) ekleme** — kullanıcı
  projede o ismin geçmesini istemiyor.

## Bilinen kısıtlar

- Latin-1 + 6 Türkçe harf dışındaki karakterler çizilmez (bkz. Dil ve font).
- DPI farkındalığı bilerek kapalı: oyun sabit sahneyi ölçekliyor, DPI-aware yapılırsa
  yüksek DPI ekranlarda pencere fiziksel olarak küçük kalır.
- Ses cihazı yoksa oyun sessiz çalışır (bilerek; `Cal()` ve `SesEfektleri::Yukle`
  içindeki savunmayı silme).
- Şirket sayısı kayıt dosyasına yazılır; şirket eklemek/çıkarmak eski kayıtları bozar.
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

## Sonraki adımlar için fikirler (istenmedi)

- v2.0'ın yeni sistemleri botlarla ve otomatik testlerle doğrulandı ama **insan
  elinde uzun süre oynanmadı**; emirlerin/kredinin oyun hissi ve rehberin temposu
  oyuncu geri bildirimiyle ayarlanmalı.
- Android: raylib destekliyor ama dokunmatik için düğmeler küçük; ayrı bir yerleşim ister.
- Müzik (prosedürel), daha fazla senaryo, günlük "tohumlu" meydan okuma.
- Web sürümünde yüksek DPI ekranlar için tuval çözünürlüğü (`devicePixelRatio`).

## Bu notların yeri

1. Bu dosya (`PROJE_NOTLARI.md`) proje kökünde, `borsa_patronu.cpp` ile aynı klasörde durur.
2. Projenin konumu: `C:\Users\Lizer\Desktop\borsa_patronu\`.
3. Notların kodlama asistanına her oturumda otomatik okutulması, depoya **girmeyen**
   yerel bir köprü dosyasıyla sağlanır (asistanın proje içindeki yerel ayar klasöründe,
   tek satır: `@../PROJE_NOTLARI.md`). O klasörü silme. Depo herkese açık olduğu için
   klasör `.git/info/exclude` ile yok sayılır ve burada adıyla anılmaz.
