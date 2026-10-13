// ============================================================================
// veri.h - oyunun ICERIK tablolari (sirketler, haberler, olaylar, rutbeler,
//          senaryolar, basarimlar, rehber). Hepsi iki dilli (IkiDil).
// ============================================================================
// Yeni icerik eklemek icin sadece bu dosyaya satir eklemek yeterli; oyun mantigi
// tablolarin uzunlugunu kendisi okur. ISTISNA: sirket sayisi kayit dosyasinin
// icine yazilir - sirket eklemek/cikarmak eski kayitlari gecersiz kilar.

// Sirketin sektoru. Bazi kuresel olaylarin belirli bir sektore EKSTRA (bazen
// ters yonlu) etkisi vardir - orn. petrol soku havayolunu fazladan vurur,
// jeopolitik gerginlik piyasayi dusururken savunmayi yukseltir.
enum class Sektor { YOK, SAVUNMA, HAVACILIK, BANKA, SANAYI, HOLDING, PERAKENDE };

struct HaberSablonu {
    IkiDil metin;
    float carpan;
};

struct SirketBilgi {
    const char* kod;
    IkiDil tamAd;
    Color renk;
    Sektor sektor;
    float baslangicFiyat;
    float beta; // genel piyasa hareketine duyarlilik (1.0 = piyasa ile ayni)
    vector<HaberSablonu> haberler;
};

// 6 hisse (RUTBELER ile ayni sayida: her rutbe atlayisinda bir yenisi acilir).
// ACILMA SIRASI baslangic fiyatina gore belirlenir (en ucuz ilk), dizideki sira
// onemli degildir. Haberin iyi mi kotu mu secilecegi havuzdaki ORANA degil
// rutbeye bagli bir olasiliga gore belirlenir (bkz. AcemiKorumasi); o yuzden
// buraya haber eklemek dengeyi degil sadece CESITLILIGI degistirir.
static const vector<SirketBilgi> SIRKETLER = {
    {"ASELS", {"Aselsan Savunma San.", "Aselsan Defense Ind."}, {31, 119, 180, 255}, Sektor::SAVUNMA, 45.0f, 0.7f, {
        {{"İhracat rekoru kırıldı!", "Export record broken!"}, 1.16f},
        {{"Tedarik zinciri sorunları yaşanıyor.", "Supply chain problems reported."}, 0.87f},
        {{"Yeni nesil radar ihalesi kazanıldı.", "Won the next-gen radar tender."}, 1.10f},
        {{"Yurt dışından büyük insansız hava aracı siparişi alındı.", "Landed a major drone order from abroad."}, 1.12f},
        {{"Savunma bütçesinde kesinti gündemde.", "Defense budget cuts are on the table."}, 0.90f},
    }},
    {"THYAO", {"Türk Hava Yolları", "Turkish Airlines"}, {214, 39, 40, 255}, Sektor::HAVACILIK, 250.0f, 1.3f, {
        {{"Turizm sezonu rekor kırdı.", "Record-breaking tourism season."}, 1.11f},
        {{"Yakıt maliyetleri sert yükseldi.", "Fuel costs have jumped."}, 0.90f},
        {{"Yeni uçuş hatları açıldı.", "New flight routes opened."}, 1.07f},
        {{"Yolcu doluluk oranı rekor seviyede.", "Passenger load factor at a record high."}, 1.09f},
        {{"Pilot grevi uçuşları aksattı.", "A pilot strike disrupted flights."}, 0.88f},
    }},
    {"GARAN", {"Garanti BBVA", "Garanti BBVA"}, {44, 160, 44, 255}, Sektor::BANKA, 60.0f, 1.05f, {
        {{"Kâr beklentisi aşıldı.", "Profit beat expectations."}, 1.09f},
        {{"Kredi faizleri yükseldi.", "Loan rates have gone up."}, 0.92f},
        {{"Kredi notu yükseltildi.", "Credit rating upgraded."}, 1.06f},
        {{"Takipteki kredi oranı geriledi.", "Non-performing loan ratio declined."}, 1.08f},
        {{"Yeni bankacılık düzenlemesi kâr marjını daralttı.", "New banking regulation squeezed margins."}, 0.91f},
    }},
    {"EREGL", {"Ereğli Demir Çelik", "Eregli Iron & Steel"}, {255, 127, 14, 255}, Sektor::SANAYI, 38.0f, 1.2f, {
        {{"Çelik ihracatı rekor kırdı!", "Steel exports hit a record!"}, 1.14f},
        {{"Demir cevheri fiyatları yükseldi, maliyetler arttı.", "Iron ore prices rose; costs are up."}, 0.89f},
        {{"Yeni yüksek fırın yatırımı devreye girdi.", "A new blast furnace came online."}, 1.09f},
        {{"Otomotiv sektöründen dev çelik siparişi.", "Huge steel order from the auto sector."}, 1.11f},
        {{"Küresel çelik fiyatları geriledi.", "Global steel prices fell."}, 0.90f},
    }},
    {"KCHOL", {"Koç Holding", "Koc Holding"}, {23, 190, 207, 255}, Sektor::HOLDING, 145.0f, 1.0f, {
        {{"Otomotiv ve enerji kollarında güçlü çeyrek sonuçları.", "Strong quarter in automotive and energy."}, 1.11f},
        {{"Kur riski holding bilançosunu baskıladı.", "Currency risk weighed on the balance sheet."}, 0.91f},
        {{"Yeni bir yurt dışı şirket alımı açıklandı.", "Announced a new overseas acquisition."}, 1.08f},
        {{"Beyaz eşya kolu ihracat rekoru kırdı.", "Home appliance arm set an export record."}, 1.09f},
        {{"Otomotiv satışları beklentinin altında kaldı.", "Auto sales fell short of expectations."}, 0.90f},
    }},
    {"BIMAS", {"BİM Mağazalar", "BIM Stores"}, {148, 103, 189, 255}, Sektor::PERAKENDE, 430.0f, 0.55f, {
        {{"Mağaza sayısı hızla artıyor, ciro rekor kırdı.", "Store count is growing fast; record revenue."}, 1.10f},
        {{"Enflasyon tüketici harcamalarını baskılıyor.", "Inflation is squeezing consumer spending."}, 0.92f},
        {{"Yeni 'BİM Online' girişimi ilgi gördü.", "The new 'BIM Online' venture drew interest."}, 1.07f},
        {{"Özel markalı ürün satışları patladı.", "Private-label sales are booming."}, 1.09f},
        {{"Rekabet Kurumu soruşturma başlattı.", "The competition authority opened a probe."}, 0.89f},
    }},
};

// ============================================================================
// YONETIM KARARLARI (bir sirketi DEVRALDIKTAN sonra periyodik olarak gelir)
// ============================================================================
struct YonetimSecenek {
    IkiDil etiket;           // buton uzerinde gorunen kisa metin
    float temelDegerCarpani; // fiyata VE adil degere uygulanan aninda etki
    float riskPrimiDegisimi; // hissenin KALICI oynaklik tabanina eklenen/cikan miktar
    IkiDil logMetni;         // haber bandina yazilacak sonuc ("ASELS YONETIM KARARI: " + bu)
};

struct YonetimOlaySablonu {
    IkiDil baslik;
    IkiDil aciklama;
    YonetimSecenek riskli;
    YonetimSecenek guvenli;
};

static const vector<YonetimOlaySablonu> YONETIM_OLAYLARI = {
    {
        {"PERSONEL KARARI", "STAFFING DECISION"},
        {"Giderleri kısmak için bir karar vermen gerekiyor.", "You need to decide how to cut costs."},
        {{"Yarısını İşten Çıkar", "Lay Off Half"}, 1.15f, 0.012f,
         {"çalışanların yarısını işten çıkardı! Kârlar arttı ama moral çökmüş durumda.",
          "laid off half its staff! Profits are up but morale has collapsed."}},
        {{"Kadroyu Koru", "Keep the Team"}, 1.02f, 0.0f,
         {"kadroyu korudu; tasarruf sınırlı ama ekip motive.",
          "kept its team; savings are limited but the staff is motivated."}},
    },
    {
        {"YATIRIM FIRSATI", "INVESTMENT OPPORTUNITY"},
        {"Şirket için yeni bir büyüme fırsatı çıktı.", "A new growth opportunity has come up."},
        {{"Agresif Genişleme", "Aggressive Expansion"}, 1.18f, 0.015f,
         {"agresif bir genişlemeye gitti! Büyüme hızlandı ama borç riski arttı.",
          "went for aggressive expansion! Growth sped up but debt risk increased."}},
        {{"Temkinli Büyüme", "Cautious Growth"}, 1.04f, 0.0f,
         {"temkinli bir büyüme seçti; istikrarlı ama yavaş ilerliyor.",
          "chose cautious growth; steady but slow."}},
    },
    {
        {"DENETİM RAPORU", "AUDIT REPORT"},
        {"İç denetimde küçük bir usulsüzlük riski tespit edildi.", "Internal audit found a small risk of irregularity."},
        {{"Örtbas Et", "Cover It Up"}, 1.10f, 0.02f,
         {"raporu örtbas etti. Kısa vadede rahatladı ama skandal riski tavan yaptı!",
          "buried the report. Short-term relief, but scandal risk has peaked!"}},
        {{"Şeffaf Ol ve Düzelt", "Come Clean and Fix It"}, 0.98f, -0.004f,
         {"şeffaflığı seçti; küçük bir maliyeti oldu ama güven arttı.",
          "chose transparency; a small cost, but trust has grown."}},
    },
    {
        {"ÜCRET GÖRÜŞMESİ", "WAGE TALKS"},
        {"Çalışanlar zam talep ediyor.", "Employees are asking for a raise."},
        {{"Talebi Reddet", "Reject the Demand"}, 1.08f, 0.01f,
         {"zam talebini reddetti. Tasarruf sağladı ama grev riski doğdu.",
          "rejected the raise. Money saved, but strike risk has emerged."}},
        {{"Zam Ver", "Give the Raise"}, 0.97f, -0.003f,
         {"çalışanlara zam verdi; maliyetli oldu ama moral yerinde.",
          "gave employees a raise; costly, but morale is high."}},
    },
    {
        {"ÜRÜN LANSMANI", "PRODUCT LAUNCH"},
        {"Yeni ürün hazır ama testleri tamamlanmadı.", "The new product is ready, but testing is not finished."},
        {{"Hemen Piyasaya Sür", "Launch Now"}, 1.14f, 0.014f,
         {"ürünü testleri bitmeden piyasaya sürdü! Satışlar patladı ama geri çağırma riski var.",
          "rushed the product to market! Sales jumped, but recall risk looms."}},
        {{"Testleri Tamamla", "Finish Testing"}, 1.03f, 0.0f,
         {"testleri tamamlamayı seçti; lansman gecikti ama ürün sağlam.",
          "chose to finish testing; the launch slipped but the product is solid."}},
    },
    {
        {"VERGİ PLANLAMASI", "TAX PLANNING"},
        {"Mali işler agresif bir vergi planı öneriyor.", "Finance is proposing an aggressive tax scheme."},
        {{"Agresif Planı Uygula", "Use the Aggressive Plan"}, 1.09f, 0.012f,
         {"agresif vergi planını uyguladı. Kâr arttı ama ceza riski büyüdü.",
          "used the aggressive tax plan. Profit rose, but penalty risk grew."}},
        {{"Kurallara Uy", "Play It Safe"}, 1.00f, -0.002f,
         {"kurallara uydu; kazanç yok ama başı ağrımayacak.",
          "played by the rules; no gain, but no trouble ahead."}},
    },
};

struct Rutbe {
    IkiDil ad;
    float esikNetDeger;
};

static const vector<Rutbe> RUTBELER = {
    {{"Acemi Tüccar", "Rookie Trader"}, 0},
    {{"Deneyimli Yatırımcı", "Seasoned Investor"}, 15000},
    {{"Uzman Trader", "Expert Trader"}, 30000},
    {{"Borsa Kaplanı", "Market Tiger"}, 60000},
    {{"Borsa Kralı", "Market King"}, 120000},
    {{"Efsane Yatırımcı", "Legendary Investor"}, 250000},
};

// ============================================================================
// KURESEL (MAKRO) OLAYLAR - tek bir sirketi degil TUM piyasayi etkiler
// ============================================================================
// temelCarpan: tum hisselere, HER BIRININ BETASIYLA olceklenerek uygulanir.
// riskPrimiDegisimi: tum hisselerin oynaklik tabanina KALICI eklenen miktar -
// oyunu zamanla zorlastiran sey budur.
struct KureselOlaySablonu {
    IkiDil metin;
    float temelCarpan;
    float riskPrimiDegisimi;
    Sektor vurgulananSektor = Sektor::YOK; // bu sektor EKSTRA etkilenir
    float sektorCarpani = 1.0f;            // genel etkinin USTUNE eklenir
};

static const vector<KureselOlaySablonu> KURESEL_OLAYLARI = {
    {{"FED faiz artırdı! Küresel piyasalarda satış dalgası başladı.",
      "The Fed raised rates! A selloff is sweeping global markets."}, 0.93f, 0.006f, Sektor::BANKA, 0.95f},
    {{"FED faiz indirdi, küresel piyasalarda risk iştahı arttı.",
      "The Fed cut rates; risk appetite is up worldwide."}, 1.06f, -0.003f, Sektor::BANKA, 1.05f},
    {{"Hürmüz Boğazı'nda gerginlik - petrol taşımacılığı durduruldu! Petrol şokunda panik.",
      "Tension in the Strait of Hormuz - oil shipping halted! Panic over an oil shock."}, 0.90f, 0.010f, Sektor::HAVACILIK, 0.93f},
    {{"Akdeniz'de bir petrol tankeri kaza yaptı, tedarik zinciri endişeleri arttı.",
      "An oil tanker crashed in the Mediterranean; supply chain worries are growing."}, 0.95f, 0.006f, Sektor::HAVACILIK, 0.96f},
    {{"Merkez Bankası sürpriz bir sıkılaşma kararı açıkladı, piyasalar temkinli.",
      "The Central Bank announced surprise tightening; markets are cautious."}, 0.96f, 0.004f},
    {{"Küresel enflasyon verileri beklentilerin altında geldi, piyasalar rahatladı.",
      "Global inflation came in below expectations; markets are relieved."}, 1.05f, -0.002f, Sektor::PERAKENDE, 1.04f},
    {{"Jeopolitik gerginlik tırmandı, yatırımcılar güvenli limana kaçıyor.",
      "Geopolitical tension escalated; investors are fleeing to safe havens."}, 0.92f, 0.008f, Sektor::SAVUNMA, 1.08f},
    {{"Büyük bir küresel ticaret anlaşması imzalandı, iyimserlik piyasaya yayıldı.",
      "A major global trade deal was signed; optimism is spreading."}, 1.06f, -0.002f, Sektor::SANAYI, 1.05f},
    {{"Küresel resesyon endişeleri piyasaları sarstı.",
      "Global recession fears shook the markets."}, 0.91f, 0.007f, Sektor::PERAKENDE, 1.04f},
    {{"Merkez Bankası faiz indirimiyle piyasaya likidite pompaladı.",
      "The Central Bank pumped in liquidity with a rate cut."}, 1.04f, -0.002f},
    {{"Kredi derecelendirme kuruluşu ülke notunu yükseltti.",
      "A rating agency upgraded the country's credit rating."}, 1.05f, -0.003f, Sektor::BANKA, 1.04f},
    {{"Döviz kurunda sert dalgalanma yaşanıyor.",
      "The currency is swinging wildly."}, 0.95f, 0.006f, Sektor::HOLDING, 0.96f},
    {{"Turizm gelirleri tüm zamanların rekorunu kırdı.",
      "Tourism revenue hit an all-time record."}, 1.04f, -0.002f, Sektor::HAVACILIK, 1.05f},
    {{"Küresel çip krizi sanayi üretimini yavaşlattı.",
      "A global chip shortage is slowing industrial output."}, 0.95f, 0.005f, Sektor::SANAYI, 0.95f},
};

// ============================================================================
// SENARYOLAR (ana menuden secilir)
// ============================================================================
// SIRA DEGISMEMELI: profil dosyasinda rekorlar bu indekslerle tutulur. Yeni
// senaryo eklerken SONA ekle.
enum SenaryoId { S_KLASIK = 0, S_KRIZ, S_ZAMAN, S_SIFIR, SENARYO_SAYISI };

struct Senaryo {
    IkiDil ad;
    IkiDil aciklama;
    IkiDil hedef;
    float baslangicBakiye;
    float baslangicRiskPrimi;     // kuresel risk primi baslangic degeri
    float baslangicPiyasaFaktoru; // piyasa modu baslangic degeri (eksi = dusus)
    bool acemiKorumasi;           // false: iyi/kotu haber orani en bastan "son rutbe" dengesinde
    int gunSiniri;                // 0 = sinir yok; >0 = bu kadar gun sonunda skor = net deger
    Color renk;
};

static const Senaryo SENARYOLAR[SENARYO_SAYISI] = {
    {{"Klasik", "Classic"},
     {"10.000 TL ile başla, rütbe atla, hisselerin kilidini aç. Oyunun standart hali.",
      "Start with 10,000 TL, rank up and unlock stocks. The standard experience."},
     {"Hedef: Efsane Yatırımcı ol", "Goal: become a Legendary Investor"},
     10000.0f, 0.0f, 0.0f, true, 0, {6, 214, 160, 255}},
    {{"Kriz Yılı", "Crisis Year"},
     {"Piyasa çöküşün ortasında açılıyor: oynaklık tavanda, kötü haber bol, acemi koruması yok.",
      "The market opens mid-crash: volatility is maxed, bad news is everywhere, no beginner protection."},
     {"Hedef: Efsane Yatırımcı ol", "Goal: become a Legendary Investor"},
     10000.0f, 0.02f, -0.012f, false, 0, {239, 71, 111, 255}},
    {{"Zamana Karşı", "Against the Clock"},
     {"Sadece 45 günün var. Süre dolduğunda net değerin skorun olur.",
      "You have only 45 days. When time runs out, your net worth is your score."},
     {"Hedef: 45 günde en yüksek net değer", "Goal: highest net worth in 45 days"},
     10000.0f, 0.0f, 0.0f, true, 45, {255, 203, 0, 255}},
    {{"Sıfırdan Zirveye", "Rags to Riches"},
     {"Cebinde sadece 2.500 TL var. Her kuruşun, her işlemin maliyeti önemli.",
      "You have just 2,500 TL. Every lira and every trading cost matters."},
     {"Hedef: Efsane Yatırımcı ol", "Goal: become a Legendary Investor"},
     2500.0f, 0.0f, 0.0f, true, 0, {17, 138, 178, 255}},
};

// ============================================================================
// BASARIMLAR (profil dosyasinda kalici; tum oyunlar arasinda ortak)
// ============================================================================
// SIRA DEGISMEMELI (profil dosyasi bu indeksleri kullanir). Yeni basarim SONA.
enum BasarimId {
    B_ILK_ISLEM = 0, B_KAR_1K, B_KAR_25K, B_PATRON, B_HOLDING, B_TEKEL, B_AYI, B_EMIR,
    B_ZARF, B_KALDIRAC, B_TEMETTU, B_DONUS, B_EFSANE, B_HIZLI, B_KRIZ, BASARIM_SAYISI
};

// "Hiz Rekoru" basarimi: Klasik senaryoda bu kadar gunden KISA surede zirve.
// Denge testinde her haberi aninda ve %85 isabetle oynayan "Haberci" botunun
// MEDYAN suresi 100 gun; yani bu basarim icin o bot kadar iyi oynamak gerekir.
// Degistirirsen asagidaki basarim aciklamasindaki sayiyi da degistir.
static const int HIZ_REKORU_GUN = 100;

struct BasarimBilgi {
    IkiDil ad;
    IkiDil aciklama;
};

static const BasarimBilgi BASARIMLAR[BASARIM_SAYISI] = {
    {{"İlk Adım", "First Step"}, {"İlk işlemini yap.", "Make your first trade."}},
    {{"Kâr Realizasyonu", "Taking Profit"}, {"Tek satışta 1.000 TL kâr et.", "Earn 1,000 TL on a single sale."}},
    {{"Büyük Vurgun", "Big Score"}, {"Tek satışta 25.000 TL kâr et.", "Earn 25,000 TL on a single sale."}},
    {{"Patron", "The Boss"}, {"Bir şirketi devral.", "Take over a company."}},
    {{"Holding", "Conglomerate"}, {"Aynı anda 3 şirketin patronu ol.", "Be the boss of 3 companies at once."}},
    {{"Tekel", "Monopoly"}, {"6 şirketin hepsini devral.", "Take over all 6 companies."}},
    {{"Ayı Avcısı", "Bear Hunter"}, {"Tek açığa satıştan 1.000 TL kâr et.", "Earn 1,000 TL from a single short."}},
    {{"Oto Pilot", "Autopilot"}, {"5 emrin (stop / kâr al / limit) gerçekleşsin.", "Have 5 orders (stop / target / limit) fill."}},
    {{"Kulağı Delik", "In the Know"}, {"Bir içeriden bilgi mektubu oku.", "Read an insider letter."}},
    {{"Kaldıraç Ustası", "Leverage Master"}, {"10.000 TL'den fazla borç al ve tamamını öde.", "Borrow over 10,000 TL and repay it all."}},
    {{"Temettü Emeklisi", "Dividend Retiree"}, {"Toplam 5.000 TL temettü topla.", "Collect 5,000 TL in dividends."}},
    {{"Küllerinden Doğan", "From the Ashes"}, {"Sermayenin %40'ını kaybettikten sonra rütbe atla.", "Rank up after losing 40% of your capital."}},
    {{"Efsane", "Legend"}, {"Efsane Yatırımcı rütbesine ulaş.", "Reach the Legendary Investor rank."}},
    {{"Hız Rekoru", "Speedrun"}, {"Klasik senaryoda 100 günden kısa sürede zirveye çık.", "Reach the top in under 100 days in Classic."}},
    {{"Kriz Fırsatçısı", "Crisis Profiteer"}, {"Kriz Yılı senaryosunda zirveye çık.", "Reach the top in the Crisis Year scenario."}},
};

// ============================================================================
// REHBER (ilk oyunun ilk dakikalari)
// ============================================================================
enum class RehberHedef { AL_BUTONU, HABER_BANDI, KART, RUTBE_CUBUGU, ADET_PANELI, YOK };

struct RehberAdimi {
    IkiDil metin;
    RehberHedef hedef;
};

static const vector<RehberAdimi> REHBER_ADIMLARI = {
    {{"Bu ilk hissen. Soldaki AL düğmesine basıp birkaç adet al - fiyat yükselirse kazanırsın.",
      "This is your first stock. Press BUY on the left to pick up a few shares - you profit if the price rises."},
     RehberHedef::AL_BUTONU},
    {{"Üstteki haber bandını izle. SON DAKİKA haberi çıktıktan 5 saniye sonra fiyata yansır. Haber iyi mi, kötü mü? Okuyup karar vermek sana kalmış.",
      "Watch the news ticker above. BREAKING news hits the price 5 seconds after it appears. Good or bad? You have to read it and decide."},
     RehberHedef::HABER_BANDI},
    {{"Kartın sağ altındaki yeşil/kırmızı rakam o pozisyondaki kâr/zararın. Kârdayken SAT'a basarak kazancını nakde çevir.",
      "The green/red number at the bottom right of the card is your profit/loss on that position. Press SELL while in profit to cash out."},
     RehberHedef::KART},
    {{"Sol üstteki çubuk bir sonraki rütbeye ne kadar kaldığını gösterir. Her rütbe yeni bir hisse ve yeni araçlar (emirler, açığa satış, kredi) açar.",
      "The bar at the top left shows how far the next rank is. Each rank unlocks a new stock and new tools (orders, short selling, credit)."},
     RehberHedef::RUTBE_CUBUGU},
    {{"Kaç adet işlem yapacağını buradan seç. Büyük ve aceleci emirler pahalıdır: 'maliyet' yüzdesine bak. Emri zamana yayarsan ucuzlar.",
      "Pick your trade size here. Large, rushed orders are expensive: watch the 'cost' percentage. Spreading an order over time makes it cheaper."},
     RehberHedef::ADET_PANELI},
    {{"Son ipucu: bir şirketin 1000 hissesinden 501'ini toplarsan patronu olursun ve yönetim kararlarını sen verirsin. Bol kazançlar!",
      "Last tip: collect 501 of a company's 1000 shares and you become its boss, making the management calls. Good luck!"},
     RehberHedef::YOK},
};
