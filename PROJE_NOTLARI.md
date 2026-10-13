# Borsa Patronu — Proje Notlari

Projenin devir/baglam dosyasi: kod, derleme, oynanis ve yayin sureciyle ilgili
bilinmesi gereken her sey burada. `borsa_patronu.cpp` ile ayni klasorde durur.

## Ne bu proje

Tek dosyalik bir C++ / raylib masaustu oyunu: Turkce bir "borsa yonetimi" is simulasyonu. Oyuncu 6 hisseyi (ASELS, THYAO, GARAN, EREGL, KCHOL, BIMAS - gercek BIST kodlari kullaniliyor ama fiyat hareketleri tamamen simule) alip satiyor, haberler (hem sirkete ozel hem TUM piyasayi etkileyen kuresel/makro olaylar) fiyatlari etkiliyor, rutbe/hedef sistemi var, hisseler kademeli olarak aciliyor (asagida), ve bir sirkette %50+1 hisseye ulasinca o sirketi devraliyor ve periyodik yonetim kararlari veriyor.

Tüm degisken/fonksiyon isimleri ve UI metinleri kasitli olarak Turkce (orijinal Gemini prototipinden miras).

## Dosyalar

- `borsa_patronu.cpp` — TEK dosyalik tum oyun (yaklasik 2400 satir). Baska hicbir .cpp/.h dosyasi yok, harici asset dosyasi yok (sesler procedural olarak kodda uretiliyor).
- `borsa_patronu.ico` + `borsa_patronu.rc` - .exe ikonu (v0.5). `.ico` HARICI BIR
  ARACLA DEGIL, `updates/` disinda tutulan kucuk bir Python betigiyle KOD ILE
  uretildi (koyu zemin + yukselen 4 cubuk, oyunun kendi paletiyle; tek dosyada
  16/24/32/48/64/128/256 px PNG girdileri). `.rc` ikonu IKI isimle birden baglar:
  `GLFW_ICON` (raylib/GLFW pencere + gorev cubugu ikonu icin bu ADI arar) ve `1`
  (Explorer'in .exe simgesi; sayisal ID isimli olandan once gelir). Ikonu
  degistirirsen iki girdiyi de birakmayi unutma. `.rc` ayrica MANIFEST'i ve
  SURUM BILGISI'ni (VERSIONINFO) de gomer - **yeni surumde `.rc` icindeki
  `FILEVERSION`/`PRODUCTVERSION`/`VALUE "FileVersion"` degerlerini `OYUN_SURUMU`
  sabitiyle BIRLIKTE guncelle**, yoksa Ozellikler penceresi eski surumu gosterir.
- `borsa_patronu.manifest` - .exe'ye gomulen Windows manifesti: `asInvoker`
  (oyun yonetici yetkisi istemez; ayrica Windows'un "bu bir kurulum programi
  olabilir" sezgisini devre disi birakir) + desteklenen OS listesi. **DPI
  farkindaligi BILEREK belirtilmedi**: oyun sabit 1280x720 sanal sahneyi
  olcekliyor, DPI-aware yapilirsa yuksek DPI ekranlarda pencere fiziksel olarak
  kucuk kalir. Bunu eklemek isteyen once olcekleme mantigini elden gecirmeli.
- `OKUBENI.txt` - dagitim paketine giren oyuncu README'si (nasil oynanir, kaydin
  nerede tutuldugu, SmartScreen uyarisi).
- `release/BorsaPatronu-vX.Y-win64.zip` - itch.io'ya yuklenen paket.
- Baska proje dosyasi yok (CMakeLists, Makefile, vs. yok — su ana kadar tek komutla derleniyordu).

## Derleme (Windows, w64devkit)

Kullanicinin bilgisayarinda raylib `C:/raylib/raylib/src` altina kurulu (raylib.com resmi installer / w64devkit ile). Calisan derleme komutu:

```
windres borsa_patronu.rc -O coff -o borsa_patronu_res.o
g++ borsa_patronu.cpp borsa_patronu_res.o -o borsa_patronu.exe -O2 -Wall -std=c++17 -I C:/raylib/raylib/src -L C:/raylib/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows -static -static-libgcc -static-libstdc++
```

**`-mwindows` BAYRAGINI SILME:** o olmadan g++ konsol (CUI) uygulamasi uretir ve
oyunun ARKASINDA siyah bir cmd penceresi acilir; dagitilan bir surumde en cok goze
batan sey budur. Dogrulama: `objdump -p borsa_patronu.exe | grep Subsystem` ->
`00000002 (Windows GUI)` yazmali (`00000003` = konsol, yanlis).
`windres` satiri ikonu .exe'ye gomer; ikon degismediyse `.o`'yu yeniden uretmeye
gerek yok. Ayrica `objdump -p ... | grep "DLL Name"` ciktisinda SADECE Windows
sistem DLL'leri gorunmeli (KERNEL32/USER32/GDI32/SHELL32/WINMM/msvcrt) - libgcc,
libstdc++ veya libwinpthread cikiyorsa `-static*` bayraklarindan biri dusmus
demektir ve oyun baska bilgisayarda acilmaz.

Bu komutu w64devkit terminalinde, `borsa_patronu.cpp` dosyasinin bulundugu klasorde calistir.

**Mevcut makinedeki kurulum (v0.4 itibariyle dogrulandi):** proje artik
`C:\Users\Lizer\Desktop\borsa_patronu` altinda ve gerekli her sey kurulu:

- **raylib 6.0**, resmi itch.io Windows installer'i ile `C:\raylib` altina kuruldu
  (`C:\raylib\raylib\src` icinde hem `raylib.h`/`raymath.h` hem `libraylib.a` var;
  ayrica installer kendi `w64devkit`'ini, `npp` ve `emsdk`'yi de getiriyor).
  Yani yukaridaki `-I/-L C:/raylib/raylib/src` yollari DOGRU, komut oldugu gibi calisiyor.
- **Iki ayri w64devkit var:** `C:\raylib\w64devkit\bin` (GCC 15.2.0, installer'dan gelen)
  ve `C:\Users\Lizer\Downloads\w64devkit\bin` (GCC 16.2.0, ayrica indirilmis).
  v0.4 build'i GCC 16.2.0 ile alindi; ikisi de sorunsuz derliyor (raylib bir C
  kutuphanesi oldugu icin `libraylib.a`'yi farkli GCC surumuyle linklemek sorun degil).
- g++ PATH'te DEGIL; derlemeden once eklenmeli, orn. bash'te:
  `export PATH="/c/Users/Lizer/Downloads/w64devkit/bin:$PATH"`
  (veya `C:\raylib\w64devkit\w64devkit.exe` terminalini acmak).
- raylib 6.0 uyum kontrolu yapildi: 6.0'da yeniden adlandirilan/kaldirilan
  fonksiyonlar (`GetMouseRay` -> `GetScreenToWorldRay`, `GetViewRay` ->
  `GetScreenToWorldRayEx`, `DrawModelPoints*`) hep 3B tarafinda; bu oyunun
  kullandigi hicbir fonksiyonu etkilemiyor. `-Wall` ile SIFIR uyariyla derleniyor. Cikti ayni klasore `borsa_patronu.exe` olarak duser, cift tiklayip oynanabilir.

Not: Kullanicinin makinesinde gercek raylib + w64devkit kurulu ve dogrulandi: `C:/raylib/raylib/src` altinda raylib kaynaklari/lib'i, `C:/raylib/w64devkit/bin` altinda g++ (bu PATH'te degil, derlemeden once `$env:PATH`'e eklenmesi gerekiyor). Yukaridaki komut bu kurulumla temiz derleniyor (uyarisiz, `-Wall` ile bile). Artik headless/sandbox test notuna gerek yok - gercek kurulumla derleyip calistirabiliyoruz.

## Su anki ozellik listesi (hepsi calisir durumda, headless test edildi)

- Gercek zamanli fiyat simulasyonu: ortak bir "piyasa faktoru" (AR(1) mean-reverting sok sureci) + hisseye ozel beta (korelasyon), "adil deger"e dogru mean-reversion, GARCH benzeri oynaklik kumelenmesi (buyuk hareket sonrasi oynaklik sicrar, yavasca soner), islem hacminin fiyata etkisi (kendi buyuk alim/satimin fiyati kendi aleyhine hareket ettirir).
- Haber sistemi: rastgele 3 sirket icin haber cikiyor, haber aninda duyuruluyor ama etkisi **5 saniye gecikmeli** uygulaniyor (`HABER_ETKI_GECIKME_SN`), ve o 5 saniye boyunca haber bandi o haberde "sabitleniyor" (kaymiyor) ki oyuncu okuyabilsin.
- Rutbe (Acemi Tuccar -> ... -> Efsane Yatirimci) ve hedef sistemi, net degere gore otomatik ilerliyor, kutlama toast/particle/ses ile.
- Ayarlar penceresi: ses ac/kapa + seviye kaydiricisi, Pencereli / Tam Ekran / Kenarsiz mod butonlari, Simge Durumuna Kucult. Artik ust barda degil, Duraklatma Menusu'nun icinde (asagida).
- Tam ekran / kenarsiz modda dogru olcekleme: sabit 1280x720 (TAM 16:9) sanal sahne bir RenderTexture'a ciziliyor, sonra gercek ekran boyutuna letterbox'li olcekleniyor. 16:9 secildigi icin 1920x1080/2560x1440/3840x2160 gibi en yaygin monitorlerde olcek TAM SAYI cikiyor (orn. 1920x1080'de 1.5x) ve SIFIR pillarbox (sag/sol siyah bosluk) kaliyor - eskiden (1280x800, 16:10) bu monitorlerde 96'sar px siyah bosluk vardi.
- Devralma mekanigi: bir sirketin 1000 hissesinden 501+ tanesini alirsan o sirketi devralirsin (`TOPLAM_HISSE_SAYISI = 1000`, esik kontrolu `DevralmaKontrolEt()` fonksiyonunda). Devraldigin sirketin kartinda altin "PATRONUSUN" rozeti cikar. Sahip oldugun surece o sirkette 14-24 saniyede bir (`YONETIM_OLAY_MIN_SN`/`MAX_SN`) rastgele bir yonetim karari penceresi aciliyor: riskli secenek (aninda fiyat/temel deger artisi ama o hisseye kalici oynaklik primi ekler, `riskPrimi` alani) vs guvenli secenek (az/hic kazanc, risk artisi yok). 4 farkli olay sablonu var (`YONETIM_OLAYLARI` vektoru: Personel Karari, Yatirim Firsati, Denetim Raporu, Ucret Gorusmesi). Hisseyi tekrar %50'nin altina dusecek sekilde satarsan kontrolu kaybedersin (bildirim cikar).
- **Kayit / Yukleme sistemi**: `OyunuKaydet()` / `OyunuYukle()` fonksiyonlari, `.exe`'nin YANINA `borsa_kayit.sav` adinda basit satir-satir bir metin formati yaziyor/okuyor (bkz. `KayitYolu()` - v0.5'te goreli yoldan `GetApplicationDirectory()`'ye gecildi; goreli yol, oyun bir KISAYOLDAN veya itch.io uygulamasindan baslatildiginda kaydi bambaska bir klasore dusuruyordu) (surum etiketi `BORSAPATRONU_SAVE_V3`; ESKI V1 dosyalari guvenle reddedilir, crash olmaz). Bakiye, gun, hisseler (fiyat/adet/temel deger/volatilite/sahiplik/risk primi/acilma seviyesi HARIC - o Baslat()'tan deterministik geliyor/fiyat gecmisi), bekleyen sirkete-ozel haberler, bekleyen KURESEL haberler, kuresel risk primi ve haber bandi gecmisi kaydediliyor. Bozuk/eksik dosya durumunda `OyunuYukle()` exception yakalayip `false` doner, oyun durumu degismeden kalir (crash olmaz). Ana menude "Kayitli Oyunu Yukle" butonu (kayit yoksa gri/pasif). Kuresel olay sayaci (`kureselHaberSayaci`) ve ses ayarlari (ac/kapa + seviye) de V3 ile birlikte kaydediliyor.

  **Kayit formatinin surum gecmisi:** V1 (3 hisse) -> V2 (6 hisse + kuresel olay
  alanlari) -> V3 (kuresel olay sayaci + ses ayarlari) -> V4 (istatistikler,
  temettu takvimi, grafik ekseni tercihi). V3, V2'ye gore SADECE
  dosyanin SONUNA alan ekledigi icin **V2 kayitlari hala yuklenebilir**:
  `OyunuYukle()` iki etiketi de kabul eder (`KAYIT_SURUM_ETIKETI` /
  `KAYIT_SURUM_ETIKETI_V2`), eksik alanlar varsayilaninda kalir ve dosya bir
  dahaki kaydediste kendiliginden V3'e yukselir. V1 hala reddedilir - onda
  ortak kisim bile uyusmuyor. **Formata yeni alan eklerken ayni yontemi kullan:**
  alani dosyanin SONUNA ekle, yeni bir surum etiketi tanimla, eskisini kabul
  edilenler arasinda birak - boylece oyuncularin mevcut kaydi silinmez.
- **Duraklatma Menusu**: ust barda artik SADECE "Duraklat" butonu var (sadelik icin). Duraklatinca ortada bir panel aciliyor: Kaydet / Ayarlar / Ana Menu / Cikis butonlari bu panelin icinde. Ana Menu, oyunu sifirlamadan (Baslat() cagirmadan) sadece ekrani Ekran::MENU yapiyor - tekrar BASLA'ya basinca kaldigi yerden devam ediyor. Cikis, ana menude de ayrica var (kirmizi CIKIS butonu). Zorunlu bir yonetim karari acikken bu menu gizli kaliyor (karardan kacilamiyor). **Panelin EN USTUNDE, "DURAKLATILDI" basliginin altinda yesil bir `Devam Et` butonu var (v0.7)**: ust bardaki Duraklat butonu karartma katmanindan ONCE cizildigi icin duraklatmada neredeyse gorunmez kaliyordu, oyuncu oyuna nasil donecegini bulamiyordu. Panel bu yuzden 400px'den 460px'e buyudu (5 buton).
- **"Yeni Oyun" (sifirlama) korumasi**: sol paneldeki `Yeni Oyun` butonu (a) DURAKLATILMISKEN artik tiklanamiyor - duraklatma paneli ekranin ORTASINDA oldugu ve bu buton SOL ALTTA kaldigi icin, karartma katmaninin altindaki butona basmak oyunu sessizce sifirlayabiliyordu; (b) iki asamali onay istiyor: ilk tik butonu `SIFIRLA_ONAY_SN` (3sn) boyunca kirmizi "EMIN MISIN? Tekrar tikla" haline getiriyor, sure dolmadan gelen ikinci tik gercekten sifirliyor (bkz. `Oyun::sifirlaOnaySuresi`). Ayarlar/yonetim penceresi acikken zaten devre disiydi.
- **Kademeli hisse acilma sistemi**: oyuna ilk girildiginde SADECE en dusuk baslangic fiyatina sahip hisse (`Hisse::acilmaSeviyesi == 0`, `Oyun::Baslat()` icinde `SIRKETLER`'in `baslangicFiyat`'ina gore siralanarak atanir) gorunur/islem yapilabilir. Her rutbe atlayisinda bir sonraki en ucuz hisse acilir (`Oyun::HisseAcikMi(idx) = rutbeIndex >= hisseler[idx].acilmaSeviyesi`). Kilitli hisseler kartlarda TAMAMEN GIZLI (haber/grafik/lejantta da gorunmuyor - `HaberKontrolEt` sadece acik hisseleri secer). 6 hisse = 6 rutbe seviyesiyle bire bir eslesiyor, oyun sonunda (Efsane Yatirimci) hepsi acik. Hisse kartlari alani sabit bir dikey alana (`KART_ALANI_YUKSEKLIK`, 3 kartlik alan) sigacak sekilde dinamik kart yuksekligi kullaniyor: 1-3 hisse acikken kartlar TAM ESKI BOYUTUNDA (130px - bkz. asagidaki 16:9 notu), 4-6 hisse acildikca "kompakt" moda geciyor (kucuk font, tamAd/mini-grafik gizli - ana grafikte zaten gorunuyor; AL/SAT butonlarinin boyutu/konumu gercek kart yuksekligine (59-95px araligi) gore dinamik).
- **Kuresel (makro) olaylar** (oyunun zorlugunu artirmak icin): sirkete ozel haberlerden ayri, TUM piyasayi birden etkileyen seyrek olaylar (`KURESEL_OLAYLARI` vektoru - FED faiz artis/indirimi, Hurmuz Bogazi krizi, Akdeniz'de petrol tankeri kazasi, Merkez Bankasi kemer sikma/genisleme, jeopolitik gerginlik, ticaret anlasmasi, vb.). `KureselHaberKontrolEt()` her `KURESEL_OLAY_TICK_SN` (10sn) saniyede bir `KURESEL_OLAY_OLASILIK` (%20) ihtimalle bir olay secer, `KURESEL_OLAY_ETKI_GECIKME_SN` (6sn) sonra `BekleyenKureselHaberleriGuncelle()` ile uygulanir (haber sistemiyle ayni "duyur, gecikmeli uygula" mantigi). Etki HER hissede kendi BETA'siyla olceklenir (yuksek beta'li THYAO gibi hisseler orantili daha cok sarsilir). Kalici etki: `Oyun::kureselRiskPrimi` - kotu olaylar bunu yukseltir, iyi olaylar biraz azaltir; TUM hisselerin oynaklik tabanina eklenir (`PiyasaGuncelle`), yani oyun ilerledikce kotu kuresel olaylar biriktikce piyasa GENEL OLARAK daha oynak/riskli hale gelir - zorluk egrisi budur. **BILEREK yesil/kirmizi renk kullanilmiyor** (toast/ekran parlamasi `RENK_UYARI` - notr turuncu): duyuru aninda haberin iyi mi kotu mu oldugu belli olmasin, oyuncu OKUYUP tahmin etmek zorunda kalsin (zorluk artirma amacli, oyuncunun kendi istegiyle eklendi). Fiyata gercekten yansidiginda (6sn sonra) kartlardaki flash yine dogru yon renginde (bu bir "spoiler" degil, gercek sonuc).

- **ISLEM MALIYETI / MIKROYAPI (v1.1) - onemli bir aciklik kapatildi**:
  Eski modelde dolum ESKI fiyattan yapilip fiyat etkisi SONRA uygulaniyordu; yani
  oyuncu KENDI ittigi fiyattan satabiliyordu. Sonuc: al-sat cevirmek RISKSIZ,
  tekrarlanabilir kar veriyordu (gercek kodla olculdu: 10.000 TL, 8 turda
  **15.926 TL**, tur basi +%6). Kodun yorumu "sonsuz bedava kar islemiyor"
  diyordu ama tam TERSI oluyordu. Yeni model:
  1. `ALIM_SATIM_SPREAD` (%0,3): her isleme alis/satis farki maliyeti. Kucuk
     islemlerde baskin maliyet budur.
  2. `TicaretEtkisi(adet)` = `TICARET_ETKI_OLCEK * sqrt(adet/TOPLAM_HISSE_SAYISI)` -
     **KAREKOK yasasi** (Almgren-Chriss / Bouchaud "square-root law"), gercek
     piyasalarda etki emir buyuklugunun karekokuyle artar. Eski DOGRUSAL model
     172 adette tavana carpiyordu, yani 250 ile 1000 adet ayni maliyetteydi ve
     buyukluk karari anlamsizdi. Adet zaten 1000 ile sinirli oldugu icin ayrica
     tavana gerek yok (1000 adet = tam %6).
  3. `DolumFiyati()`: dolum piyasa fiyatinin alimda USTUNDE, satista ALTINDA
     (`spread/2 + etki * ETKI_DOLUM_PAYI`). Piyasa fiyati ayrica TAM etki kadar
     kayar - emir defterinde "yuruyorsun", ortalama etkinin yarisini odersin.
  Olculen sonuc (gercek derlenmis fonksiyonlarla): ayni 8 tur artik
  **10.000 -> 9.733 TL**. Tek islem maliyeti 1 adette %0,25, 1000 adette %3,15.
  `IslemAdedi()` MAKS hesabi da dolum fiyatina gore ITERATIF yapiliyor (dolum
  adede bagli oldugu icin "bakiye/fiyat" yanlis cevap verir).
  **Maliyet oyuncuya GORUNUR**: adet panelinde secili adet icin `maliyet ~%1,65`
  yaziyor, haber bandinda dolum fiyati + maliyet yuzdesi geciyor. Gorunmez vergi
  gibi hissettirmemesi icin bu sart.
  **AYAR NOTU:** dengeyi degistirecekseniz `ALIM_SATIM_SPREAD` tur basi cevirme
  maliyetini, `TICARET_ETKI_OLCEK` ise buyuk pozisyon almanin maliyetini belirler.
- **Arayuz cakisma duzeltmeleri (v1.0)** - v0.9 paketi oyunda denenince uc
  cakisma cikti, ucu de duzeltildi:
  1. `Eksen: TL/%` butonu grafik panelinin SAG ustundeydi ve sag hizali
     "PIYASA MODU: ..." yazisinin (en uzun hali ~330px) TAM UZERINE biniyordu.
     Buton, panel basliginin sagindaki bos alana alindi (`grafikPanel.x + 250`).
     **Yeni buton koyacaksan panelin sag ustu DOLUDUR, oraya koyma.**
  2. `Secili: MAKS (bakiye/elindeki kadar)` yazisi adet panelinin sag kenarindan
     58px tasiyordu; `Secili: MAKS` olarak kisaltildi.
  3. Kart pozisyon K/Z'si ile soldaki "Elde: N adet  Deger: X TL" yazisi buyuk
     rakamlarda (orn. 1000 adet BIMAS) UST USTE BINIYORDU. Artik ikisinin
     genisligi `MeasureText` ile OLCULUP karsilastiriliyor; sigmiyorsa K/Z
     kisa forma (`+13,6%`) dusuyor. Kompakt kartta da ayni koruma var.
  Ayrica **PORTFOY OZETI paneli** eklendi: kart alani her zaman 3 kartlik yer
  ayirdigi icin 1-2 hisse acikken altta 100-250px bos bolge kaliyordu; oraya
  pozisyon degeri / nakit / gerceklesen K/Z / islem sayisi konuldu (3+ hisse
  acilinca yer kalmadigi icin kendiliginden gizlenir).

  **NASIL DOGRULANDI (bu yontemi tekrar kullan):** kaynagin bir KOPYASI
  `scratchpad`'e alinip main()'e kucuk bir test kancasi eklendi (oyun dogrudan
  OYUN ekraninda basliyor, sahte portfoy/rutbe kuruluyor, belirli karelerde
  `LoadImageFromTexture(sahneDokusu.texture)` + `ImageFlipVertical` + `ExportImage`
  ile PNG yaziliyor, sonra cikiyor). Boylece elle oynamadan 1/3/6 hisse, normalize
  grafik ve duraklatma ekranlari gercek render'dan dogrulanabiliyor.
  DIKKAT: `TakeScreenshot()` `EndDrawing()`'den ONCE cagrilirsa SIYAH kare verir
  (tampon hazir degil) - sahne dokusunu dogrudan disa aktarmak gerekiyor.
- **UI/oynanis paketi (v0.9)** - tek seferde eklenen 9 iyilestirme:
  1. **Kartlarda pozisyon kar/zarari**: ortalama maliyet (`h.maliyet/h.adet`) zaten
     tutuluyordu ama gosterilmiyordu; kart sag alt kosesinde `+%12,4 (+320 TL)`
     olarak yesil/kirmizi cikiyor (kompakt kartta sadece yuzde).
  2. **MAKS ve 250 adet secenegi**: adet paneli 6 butondan 8'e cikti (3 sutun ->
     4 sutun, panel yuksekligi AYNI kaldi). `secilenAdet == -1` MAKS demek:
     alimda bakiyenin yettigi kadar, satista elindekinin TAMAMI (bkz.
     `IslemAdedi()`). 501 hisselik devralma esigi artik onlarca tik istemiyor.
  3. **Normalize (yuzde) grafik**: grafik panelinin sag ustundeki `Eksen: TL / %`
     butonu. Yuzde modunda her hisse kendi gecmis penceresinin ILK degerine gore
     cizilir; 430 TL'lik BIMAS ile 38 TL'lik EREGL karsilastirilabilir hale gelir
     (mutlak eksende ucuz hisseler duz cizgiye donuyordu).
  4. **Klavye kisayollari**: `Bosluk`/`P` duraklat, `1-8` islem adedi sec,
     `M` sesi ac/kapa. Modal pencere (ayarlar/yonetim/cikis onayi) acikken pasif.
  5. **Rutbe cubugunda rakam**: cubugun yaninda `142.300 / 250.000 TL`.
  6. **Temettu**: her `TEMETTU_PERIYOT_GUN` (5 gun ~ 90 sn) gunde bir, portfoyun
     guncel degerinin `TEMETTU_ORANI` (%0,5) kadari nakit gelir. Uzun vadeli
     tutmayi gecerli bir strateji yapar.
  7. **Sektorel capraz etkiler**: `Sektor` enum'u (SAVUNMA/HAVACILIK/BANKA/
     SANAYI/HOLDING/PERAKENDE) her sirkete atandi. Kuresel olaylara opsiyonel
     `vurgulananSektor` + `sektorCarpani` eklendi: petrol soku HAVACILIK'i
     fazladan vurur, jeopolitik gerginlik piyasayi dusururken SAVUNMA'yi
     YUKSELTIR (ters yonlu etki mumkun). Etki genel etkinin USTUNE eklenir.
     NOT: `BeklemedeKuresel` icindeki sektor alanlari KAYDA YAZILMAZ (tekrarli
     blogun ortasina alan eklemek eski kayitlari bozardi) - yuklenen bekleyen
     olay sektorel vurgusunu kaybeder, genel etkisi aynen uygulanir.
  8. **Istatistikler**: islem sayisi, gerceklesen (realize) kar/zarar, en iyi ve
     en kotu satis. Ayri ekran yerine DURAKLATMA panelinin altinda gosteriliyor -
     herkesin kesin gordugu yer orasi.
  9. Kayit formati **V3 -> V4** (istatistikler + temettu takvimi + grafik ekseni
     tercihi). V4/V3/V2 hepsi yuklenebilir, eski kayitlar bozulmaz.
- **Iceriden bilgi (insider) mektubu (v0.8)**: rutbe `INSIDER_MIN_RUTBE` (3 =
  "Borsa Kaplani") ve ustunde, her `INSIDER_TICK_SN` (20sn) saniyede
  `INSIDER_OLASILIK` (%10) ihtimalle ust barda (Duraklat butonunun SOLUNDA) altin
  bir ZARF belirir. Tiklarsan bir hissenin YAKIN GELECEKTE yuzde kac yukselecegini
  soyler (`INSIDER_ARTIS_MIN/MAX` = %8-22), yukselis `INSIDER_GECIKME_MIN/MAX_SN`
  (12-20sn) sonra gerceklesir - yani once pozisyon alma sansin olur.
  **KRITIK TASARIM NOTU:** yukselis, mektup OLUSTUGU anda `bekleyenHaberler`
  kuyruguna eklenir; yani mektubu okusan da okumasan da GERCEKLESIR. Mektup olayi
  yaratmiyor, sadece onceden haber veriyor - okumazsan sadece firsati kacirirsin.
  Zarf `INSIDER_MEKTUP_OMUR_SN` (12sn) sonra kaybolur (altinda kalan sureyi
  gosteren ince cubuk var). Okunduktan sonra grafik panelinin sag ustunde,
  yukselis gerceklesene kadar duran bir geri sayim seridi cikar. Bilgi haber
  bandina BILEREK yazilmaz - bu sadece OYUNCUYA gelen ozel bir bilgi.
  Zarf ikonu raylib'in varsayilan fontunda glyph olmadigi icin (ASCII 224 karakter)
  ilkel sekillerle ciziliyor. Mektup durumu KAYDEDILMEZ (gecici, en fazla 20sn);
  ama planlanmis yukselis `bekleyenHaberler` icinde oldugu icin kayittan sonra da
  gerceklesir - yani kaydet/yukle ile ne kacirilir ne de suistimal edilir.
- **Acemi korumasi / zorluk egrisi (v0.6)**: olay havuzlarina DOKUNULMADAN, olayin
  iyi mi kotu mu olacagi RUTBEYE gore agirliklandiriliyor (`AcemiKorumasi()` +
  `AgirlikliOlaySec()`). Sebep olculdu: kuresel havuzda 10 olayin 6'si kotuydu VE
  kotu olaylar iyilerden sert vuruyordu (ort. -%7,2'ye karsi +%5,3), yani olay
  basina -%2,2 beklenen kayip; olaylar ~50 sn'de bir geldigi icin kuresel risk
  primi daha ~5 DAKIKADA tavana vuruyor, oyun en oynak haline aninda geliyordu.
  Yeni egri (kuresel olaylarda iyi olasiligi): rutbe 0 %75 -> rutbe 5 %40.
  Sirket haberlerinde: %85 -> %67. **Her ikisinin de SON rutbedeki degeri eski
  davranisla BIREBIR AYNI** - oyunun zor sonu korundu, sadece giris yumusatildi.
  Rutbe 0-2 (ilk 3 hisse acikken) beklenen deger ARTIDA, rutbe 3'ten sonra eksiye
  doner. Ayar: `ACEMI_*_TABAN` (son rutbe degeri) ve `ACEMI_*_BONUS` (rutbe 0'da
  eklenen). TABAN'lari degistirirsen oyunun ZOR SONUNU degistirmis olursun.
- **Cikis onayi (v0.5)**: pencereyi X ile kapatmak, ana menudeki CIKIS ve duraklatma
  menusundeki Cikis artik dogrudan cikmiyor; en az bir islem yapilmissa
  (`ilkIslemYapildi`) ortada "Kaydet ve Cik / Kaydetmeden Cik / Vazgec" penceresi
  aciliyor (`Oyun::cikisOnayiAcik`). Ana dongunun kosulunda ARTIK `WindowShouldClose()`
  YOK - istek her karede elle okunuyor; raylib'in `WindowShouldClose()`'u her
  cagrildiginda GLFW bayragini sifirladigi icin istegi "yutup" oyunu acik tutmak
  guvenli. Onay penceresi acikken simulasyon donuyor ve `tiklandi` false'a cekilerek
  tikin alttaki arayuze gecmesi engelleniyor.
- **Zirve / oyun sonu (v0.5)**: son rutbeye (Efsane Yatirimci) ulasinca ozel bir
  kutlama (toast + haber bandi + parcacik) ve ust bardaki ilerleme cubugunun yaninda
  "ZIRVE" etiketi cikiyor. Oyun KAPANMIYOR - oyuncu kum havuzu olarak devam edebilir.
  Rutbe hicbir zaman DUSMEDIGI icin bu blok bir oyunda en fazla bir kez calisir.
- **Surum numarasi (v0.5)**: `OYUN_SURUMU` sabiti ana menunun sag alt kosesinde
  gosteriliyor. **Her yayin oncesi `updates/vX.Y` ile AYNI degere guncelle** - itch.io'da
  gelen geri bildirimleri dogru surumle eslestirebilmenin tek yolu bu.

## Piyasa simulasyonunun matematigi (olculmus degerlerle)

Her saniye (`PIYASA_TICK_SN`) her hisse icin: `r = e*sigma_v + beta*M - 0.05*d`,
sonra `P = max(0.5, P*(1+r))`.

- `RastgeleGauss()` = 4 tekduze [-1,1] ortalamasi. **std sapmasi 1 DEGIL, 1/sqrt(12)
  = 0,289**; yani `volatilite = 0,008` yazdiginda gercek sigma 0,0023'tur. Sabit
  ayarlarken bu carpani unutma. Ayrica |x| asla 1'i gecemez - kuyruklar KESILI,
  tek tikte fiyat ucamaz (bilincli tasarim).
- **Piyasa faktoru** AR(1): `M = 0.965*M + gauss*0.006`. Yari-omur
  ln(0,5)/ln(0,965) = **19,5 saniye**; durağan std sapma **%0,67**. PIYASA MODU
  gostergesinin neden onlarca saniye ayni kaldigi bu.
- **Beta / korelasyon** (olculdu): tik basi std BIMAS %0,33 -> THYAO %0,64.
  Varyansin ortak faktorden gelen payi **%72 (BIMAS) - %92 (THYAO)**; gerceklesen
  korelasyonlar 0,57-0,69. **NOT: bu gercek borsalardan YUKSEK** (gercekte tek
  hisse varyansinin ~%30-50'si piyasadir) - yani cesitlendirme bu oyunda az
  koruyor. Dusurmek icin `MARKET_SOK_BUYUKLUK`'u kisip `BASE_VOLATILITE`'yi artir.
- **Adil degere cekim**: sapmanin yari-omru ln(0,5)/ln(0,95) = 13,5 sn. Ama piyasa
  faktoru 19,5 sn'lik yari-omurle ayni yone ittigi icin itis cekisten inatci;
  olculen tipik sapma BIMAS ±%5,7, THYAO ±%13,3 (1 sigma). Fiyatlar adil degere
  yapismiyor, etrafinda genis yorungeler ciziyor - istenen davranis bu.
- **Oynaklik kumelenmesi** tetiklenme sikligi betaya cok duyarli: BIMAS'ta pratikte
  hic (%0,00), THYAO'da tiklerin %0,53'unde. Dusuk betali hisseler kendi baslarina
  firtinaya girmez, onlari sadece haber/kuresel olay sarsar.

## Bilinen kisitlamalar / dikkat edilecekler

- Font sadece raylib'in varsayilan bitmap fontu (224 glyph) - Unicode karakterler (★, — gibi) render olmuyor, tofu kutusu cikiyor. Bu yuzden kodda sadece ASCII karakterler kullanildi (★ yerine `|`, — yerine `-`). Yeni metin eklerken bu kisitlamaya dikkat et.
- Turkce'ye ozgu karakterler (ı, ş, ğ, ü, ö, ç) de ayni sebeple KULLANILMADI - butun metinler "Turkce ASCII" (sadece + kucuk d + s + vs. yerine duz harfler) yaziliyor (orn. "Duraklat", "Sirket" degil, mevcut kod boyle). Tutarliligi bozma.
- Ses efektleri harici dosya degil, kod icinde sinus dalgasi + fade envelope ile prosedurel olarak uretiliyor (`LoadSoundFromWave`). Yeni ses eklemek istersen ayni yontemi kullan, disaridan .wav yuklemeye gerek yok (ve olmadigi icin proje "asset-free" kaliyor).
- Ses cihazi bulunamazsa (`IsAudioDeviceReady()` false donerse) oyun sessiz calismaya devam ediyor, crash olmuyor - bu kasitli bir savunma, silme.
- `Clamp` ve `Lerp` fonksiyonlari `raymath.h`'de - raylib.h'in kendisinde degil, o yuzden dosyanin basinda `#include "raymath.h"` var, kaldirma.

## Su ana kadar KULLANICI TARAFINDAN istenen ozellik gecmisi (siralama onemli)

1. Orijinal Gemini Tkinter prototipini iyilestirme istegi (bu asama artik gecerli degil, proje C++'a tasindi).
2. Haberlerin fiyati aninda degil 5 saniye gecikmeli etkilemesi.
3. Tam bir C++ (raylib) yeniden yazimi + "is yonetimi oyunu" hissi (juice/animasyon).
4. Haber bandinin en yeni haberi okunabilir sekilde sabitlemesi (5 saniyelik gecikme boyunca).
5. Duraklat butonunun yanina Ayarlar menusu (ses + pencere modu).
6. Tam ekranda ekranin bir kismi siyah kalma hatasinin duzeltilmesi (olcekleme).
7. Piyasa simulasyonunun daha gercekci ama hala ongorulemez olmasi.
8. Devralma / yonetim mekanigi.
9. Devralma/yonetim penceresinin haber bandi metni ve rozetin kucuk ekranlarda tasip tasmadiginin gercek raylib kurulumunda dogrulanmasi (dogrulandi, tasma yok - bkz. `updates/v0.1` oncesi konusma gecmisi).
10. Kayit / Yukleme sistemi: ana menude "Yukle" butonu, oyun icinde "Kaydet" butonu.
11. Ana menude ve oyun icinde Cikis butonu + oyun icinde Ana Menuye Don butonu.
12. Sadelik icin ust bar butonlarinin (Kaydet/Ayarlar/Ana Menu/Cikis) Duraklatma Menusu altinda toplanmasi - ust barda artik sadece Duraklat kaliyor.
13. Kademeli hisse acilma sistemi: oyuna ilk girildiginde sadece en dusuk fiyatli hisse acik, her rutbe atlayisinda bir yenisi aciliyor. Bu vesileyle 3 yeni sirket eklendi (EREGL, KCHOL, BIMAS), toplam 6 hisse = 6 rutbe seviyesi.
14. Oyunun zorlugunu artirmak icin kuresel/makro olaylar: tek sirketi degil TUM piyasayi etkileyen seyrek olaylar (FED faiz karari, Hurmuz Bogazi krizi, petrol tankeri kazasi, Merkez Bankasi politikalari, vb.), kalici bir "kuresel risk primi" biriktirip piyasayi zamanla daha oynak hale getiriyor.
15. Kuresel haber bildirimlerindeki yesil/kirmizi renklendirmenin kaldirilmasi - oyuncu haberin iyi mi kotu mu oldugunu onceden bilmesin, oyun zorlassin diye (bilerek istendi, "kolaylik" degil).
16. Ekran cozunurlugu 1280x800'den (16:10) 1280x720'ye (TAM 16:9) degistirildi - 1920x1080 gibi yaygin monitorlerde tam ekran/kenarsiz modda kalan sag/sol siyah bosluklarin (pillarbox) giderilmesi icin. Sol paneldeki hisse kartlari/adet secim paneli/Yeni Oyun butonu bu 80px'lik azalmaya gore yeniden olceklendi.
17. Kod incelemesi sonrasi hata/tutarlilik duzeltmeleri (v0.4): duraklatmada
    "Yeni Oyun" butonunun hala tiklanabilir olmasi (istemeden sifirlama), sifirlamaya
    iki asamali onay, kayit dosyasinda eksik olan `kureselHaberSayaci` + ses ayarlari
    (format V2 -> V3, V2 geriye donuk uyumlu), ana menude iki metnin ust uste yazilmasi,
    olu kod temizligi ve eskimis yorum/dokuman notlari.
18. Oyunun BASI cok zor geldigi geri bildirimi (v0.6): "cok fazla kotu olay
    oluyor, iyi olay az, oyuncu erken sikilabilir - ilk acilan 3 hisse icin daha
    cok iyi haber gorelim". Cozum: havuzlari degistirmek yerine rutbeye bagli
    agirliklandirma (bkz. Acemi korumasi). Son rutbe eskisiyle ayni birakildi.
19. Duraklatma menusunde "Devam Et" butonunun panelin en ustune, "DURAKLATILDI"
    basliginin altina alinmasi (v0.7) - ust bardaki buton karartma altinda kalip
    gorunmuyordu.
20. Iceriden bilgi (insider) mektubu (v0.8): ust seviyelerde nadiren gelen,
    tiklaninca bir hissenin yakin gelecekteki yukselisini ve yuzdesini gosteren
    zarf ikonu.
21. "Yapabildiginin hepsini yap" (v0.9): UI/oynanis onerilerinin TAMAMI
    uygulandi - yukaridaki 9 maddelik paket.
22. Ekran goruntusuyle gelen "her sey birbirine girmis" geri bildirimi (v1.0):
    uc arayuz cakismasi olculup duzeltildi, bos kart alanina portfoy ozeti eklendi.
23. "Piyasa simulasyonu matematigini aciklar misin" (v1.1): aciklama sirasinda
    al-sat dongusu acikligi bulundu; kullanici "duzelt, piyasa en gercekci ama
    ayni zamanda oyun oldugunu hissettiren sekilde olsun" dedi -> mikroyapi
    (spread + karekok etki + dolum fiyati) bastan yazildi.

## Yayinlama (itch.io)

Dagitim paketi `release/BorsaPatronu-vX.Y-win64.zip`: SADECE `borsa_patronu.exe`
ve `OKUBENI.txt`. Paket hazirlarken:

1. `-mwindows` ile ve `windres` ikonu baglanmis sekilde derle (bkz. Derleme).
2. `objdump -p` ile Subsystem'in `00000002 (Windows GUI)` ve DLL listesinin sadece
   sistem DLL'leri oldugunu dogrula.
3. **`borsa_kayit.sav` DOSYASINI ZIPE KOYMA.** Klasoru oldugu gibi sikistirirsan
   kendi oyun kaydin oyuncuya gider ve oyun senin ortandan baslar.
4. `updates/`, `.cpp`, `.ico`, `.rc` da pakete GIRMEZ - oyuncuya sadece calisan
   .exe ve README gider.
5. **SmartScreen ("Bilinmeyen yayimci") uyarisi hakkinda:** bu bir VIRUS TESPITI
   DEGIL, itibar (reputation) kontroludur - imzasiz ve az indirilmis her .exe alir.
   Kalici cozumu yok denecek kadar azdir: Mart 2024'te Microsoft, EV sertifikalarin
   "aninda itibar" ayricaligini KALDIRDI; artik OV ve EV ayni sekilde, indirme
   hacmiyle zamanla itibar kazaniyor. Yani sertifikaya para vermek bile ilk gunku
   uyariyi kaldirmiyor. Microsoft'un ucuz servisi Azure Trusted Signing bireysel
   gelistiriciler icin sadece ABD/Kanada'da acik (Turkiye'den olmuyor).
   Yapilabilecekler: (a) .exe'ye surum bilgisi + manifest gommek (v0.5'te yapildi),
   (b) SIK SIK yeni build yayinlamamak - SmartScreen itibari dosya HASH'ine bagli,
   her yeni .exe itibari sifirlar, (c) itch.io sayfasinda ve OKUBENI.txt'te durumu
   acikca yazmak. Defender GERCEK bir tehdit adi verirse (orn. Trojan:Win32/Wacatac)
   o zaman durum farklidir: yanlis pozitiftir ve Microsoft'a bildirilmelidir.
6. **Sayfa gorselleri** `sayfa_gorselleri/` altinda (pakete GIRMEZ, sadece itch.io
   sayfasina yuklenir): `kapak_630x500.png` zorunlu kapak gorseli - `kapak_uret.py`
   ile KOD ILE uretiliyor (oyunun paleti + Windows fontlari), degistirmek istersen
   o betigi duzenleyip tekrar calistir. Kapak 315:250 oraninda olmali (min 315x250).
   Ekran goruntuleri: oyun icinde **F12** tusu calisir - raylib'in `SUPPORT_SCREEN_CAPTURE`
   ozelligi acik oldugu icin ek kod GEREKMEDI; goruntuler .exe'nin calisma dizinine
   `screenshot000.png` diye duser. 3-5 tane onerilir.
7. itch.io yuklemesinde dosyayi "Windows" olarak isaretle (itch uygulamasinin
   kurabilmesi icin) ve `.exe`'nin imzasiz oldugunu, SmartScreen uyarisinin
   normal oldugunu sayfada belirt (`OKUBENI.txt` icinde de yaziyor).

## Versiyonlama (updates/ klasoru)

Kullanicinin istegiyle, her anlamli guncelleme tamamlandiginda proje kok
klasorundeki calisan dosyalarin (`borsa_patronu.cpp`, `borsa_patronu.exe`,
`PROJE_NOTLARI.md`) bir kopyasi `updates/vX.Y/` altina alinir (ornek: `updates/v0.1/`).
Bu proje bir git deposu DEGIL; versiyonlama tamamen bu manuel klasor
kopyalamasiyla yapiliyor.

- **Calisan/guncel dosyalar HER ZAMAN proje kok klasorunde** (su an `C:\Users\Lizer\Desktop\borsa_patronu\`)
  kalir - derleme/calistirma hep buradan yapilir. `updates/` klasoru sadece
  gecmis snapshot'lari saklar, oradan calistirilmaz.
- Yeni bir guncelleme (ozellik/duzeltme seti) tamamlanip kullanici onayladiginda:
  1. Bir sonraki versiyon numarasini belirle (`v0.1` -> `v0.2` gibi, kullaniciya sor gerekirse).
  2. `updates/vX.Y/` klasoru olustur.
  3. Guncel `borsa_patronu.cpp`, `borsa_patronu.exe`, `PROJE_NOTLARI.md` dosyalarini oraya kopyala (tasima degil, kopyalama - kok klasordeki dosyalar oldugu gibi kalir).
- `borsa_kayit.sav` (oyuncu kayit dosyasi) versiyon snapshot'larina DAHIL EDILMEZ - bu bir oyun kaydi, kod/proje versiyonu degil.
- Su ana kadarki versiyonlar:
  - `updates/v0.1/` - kayit/yukleme sistemi + ana menu/oyun ici Cikis butonlari + Duraklatma Menusu (Kaydet/Ayarlar/Ana Menu/Cikis konsolidasyonu) tamamlandiktan sonraki ilk snapshot.
  - `updates/v0.2/` - kademeli hisse acilma sistemi (6 hisseye cikildi: EREGL/KCHOL/BIMAS eklendi) + kuresel/makro olaylar sistemi (kayit formati V1->V2).
  - `updates/v0.3/` - kuresel haber renklerinin (yesil/kirmizi) kaldirilmasi (zorluk artirma) + cozunurluk 1280x800'den 1280x720'ye (16:9) degisimi, 1920x1080 gibi monitorlerde pillarbox'i gidermek icin.
  - `updates/v0.4/` - kod incelemesi sonrasi duzeltme paketi: duraklatmada "Yeni Oyun"
    butonunun devre disi birakilmasi + iki asamali sifirlama onayi, kayit formati
    V2 -> V3 (kuresel olay sayaci ve ses ayarlari eklendi; V2 kayitlari yuklenmeye
    devam ediyor), ana menudeki metin cakismasi, olu kod (`basbtonu`, kullanilmayan
    `merkez`) temizligi ve eskimis yorumlarin/dokuman notlarinin duzeltilmesi.
    Snapshot tam (`cpp` + `exe` + `PROJE_NOTLARI.md`); build raylib 6.0 + GCC 16.2.0 ile,
    `-Wall` altinda sifir uyariyla alindi.
  - `updates/v1.1/` - islem maliyeti/mikroyapi yeniden yazimi: al-sat dongusu
    acikligi kapatildi, karekok etki yasasi + spread + dolum fiyati.
  - `updates/v1.0/` - v0.9'un arayuz cakismalarinin duzeltilmesi + portfoy ozeti
    paneli. Gercek render karelerinden gozle dogrulandi.
  - `updates/v0.9/` - UI/oynanis paketi (kart K/Z, MAKS adet, normalize grafik,
    klavye kisayollari, rutbe rakami, temettu, sektorel etkiler, istatistikler);
    kayit formati V4.
  - `updates/v0.8/` - iceriden bilgi (insider) mektubu mekanigi.
  - `updates/v0.7/` - duraklatma menusune gorunur `Devam Et` butonu (panel 460px).
  - `updates/v0.6/` - erken oyun zorluk dengesi (acemi korumasi) + itch.io sayfa
    materyalleri (`kapak_uret.py` ile uretilen kapak gorseli, `itch_sayfa_metni.md`).
    YAYINLANAN ILK SURUM BU. (v0.5 hic yayinlanmadi, sadece hazirlik snapshot'i.)
  - `updates/v0.5/` - yayin hazirligi. Cikis hazirligi paketi:
    `-mwindows` (arkadaki konsol penceresi kaldirildi), .exe ikonu (`ico`+`rc`),
    kayit dosyasinin `.exe` yanina sabitlenmesi (`KayitYolu()`), cikis onayi
    penceresi (X / CIKIS / duraklatma menusu), zirve (oyun sonu) kutlamasi ve
    ana menude surum numarasi. Ayrica `OKUBENI.txt` ve `release/` zip'i eklendi.

## Onerilen sonraki adimlar (kullanici henuz istemedi ama dogal devam noktalari)

- **v0.4 OYUN ICI DOGRULAMASI (derleme yapildi, elle test HENUZ yapilmadi):**
  `borsa_patronu.exe` guncel koddan uretildi ama oyun acilip denenmedi. Bir kez calistirip
  (a) duraklatip sol alttaki "Yeni Oyun" alanina tiklayinca sifirlanmadigini,
  (b) tek tikin butonu "EMIN MISIN?" yaptigini ve 3sn sonra normale dondugunu,
  (c) mevcut `borsa_kayit.sav` (V2) dosyasinin sorunsuz yuklendigini,
  (d) kaydedince dosyanin ilk satirinin `BORSAPATRONU_SAVE_V3` oldugunu dogrula.

- Ses efektlerinin cesitlendirilmesi (ozellikle kuresel olaylar icin ayri bir "buyuk haber" sesi olabilir).
- Windows .exe icin CI/otomatik derleme scripti yok - her seferinde manuel `g++` komutu calistiriliyor.
- Kuresel olaylarin sektore ozel/asimetrik etkileri simdilik yok (hepsi TUM hisseleri ayni yonde, sadece beta agirlikli etkiliyor) - istenirse "petrol sokunun THYAO'yu ekstra vurmasi" gibi sektorel carpanlar eklenebilir.

## Bu notlarin yeri

1. Bu dosya (`PROJE_NOTLARI.md`) `borsa_patronu.cpp` ile AYNI klasorde durur;
   projeye yeni oturan herkesin (insan ya da yapay zeka asistani) once okumasi
   gereken tek dosya budur.
2. Projenin su anki konumu: `C:\Users\Lizer\Desktop\borsa_patronu\`.
3. Bir kodlama asistani kullaniyorsan klasoru ac ve bu dosyayi okumasini iste;
   derleme komutu, bilinen kisitlamalar ve yayin sureci hepsi burada yazili.
4. Yerel asistan ayar klasorundeki tek satirlik kopru dosyasini SILME. Icerigi sadece
   `@../PROJE_NOTLARI.md` satiridir ve kullanilan kodlama asistaninin bu notlari
   her oturumda OTOMATIK okumasini saglar. Notlarin kendisi bilerek proje
   kokunde ve tarafsiz isimle (`PROJE_NOTLARI.md`) tutuluyor; kopru silinirse
   dosya durmaya devam eder ama asistana her seferinde elle okutman gerekir.
