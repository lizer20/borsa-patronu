/*
 * ============================================================================
 *  BORSA PATRONU  -  Trading Tycoon
 * ============================================================================
 *  C++17 + raylib ile yazilmis, Turkce/Ingilizce bir borsa yonetim oyunu.
 *
 *  DOSYA DUZENI ("unity build"): derlenen TEK dosya budur; oyunun parcalari
 *  asagidaki sirayla buraya dahil edilir. Yani derleme komutu hala tek satir,
 *  ama kod konusuna gore ayrilmis durumda:
 *
 *    src/surum.h            surum numarasi (tek kaynak; .rc de bunu okur)
 *    src/temel.h            DENGE SABITLERI, renk paleti, matematik yardimcilari
 *    src/platform.h         masaustu/web farklari (dosya <-> localStorage)
 *    src/dil.h              iki dilli metin, Turkce harfli font, yazi cizimi
 *    src/ses.h              prosedurel ses efektleri
 *    src/efekt.h            parcacik, toast, buton, ortak cizim yardimcilari
 *    src/veri.h             ICERIK tablolari (sirket, haber, olay, senaryo...)
 *    src/oyun.h             oyun durumu (veri yapilari)
 *    src/profil.h           basarimlar, rekorlar, dil tercihi
 *    src/piyasa_islem.h     alim/satim ve islem maliyeti (mikroyapi)
 *    src/piyasa_kaldirac.h  emirler, kredi, teminat cagrisi, iflas
 *    src/piyasa_olay.h      fiyat simulasyonu, haberler, bilanco, rutbe
 *    src/piyasa_sim.h       gun dongusu ve SimulasyonAdimi()
 *    src/kayit.h            kaydet / yukle
 *    src/arayuz_temel.h     arayuz veri yapilari ve sabit YERLESIM
 *    src/arayuz_oyun.h      ust bar, haber bandi, hisse kartlari, adet paneli
 *    src/arayuz_grafik.h    ana grafik, rehber katmani, oyun alani girdisi
 *    src/arayuz_pencere*.h  pencereler (duraklatma, ayarlar, emir, kredi...)
 *    src/arayuz_menu.h      ana menu, senaryo secimi, basarimlar
 *    src/uygulama.h         pencere, girdi ve kare dongusu
 *
 *  DERLEME:  python build.py        (ayrintilar PROJE_NOTLARI.md icinde)
 *
 *  MIMARI NOT: oyun mantigi (piyasa_*.h) hicbir pencere/cizim fonksiyonu
 *  cagirmaz. Bu sayede test/denge_testi.cpp ayni kodu pencere ACMADAN
 *  calistirip botlara binlerce oyun oynatabilir. Arayuz ise her kareyi
 *  KareGuncelle() + KareCiz() olarak isler; test/gorsel_test.cpp bu iki
 *  fonksiyonu sahte fare girdisiyle surup ekran goruntusu alir.
 * ============================================================================
 */

#include "raylib.h"
#include "raymath.h" // Clamp(), Lerp()

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#include "src/surum.h"
#include "src/temel.h"
#include "src/platform.h"
#include "src/dil.h"
#include "src/ses.h"
#include "src/efekt.h"
#include "src/veri.h"
#include "src/oyun.h"
#include "src/profil.h"
#include "src/piyasa_islem.h"
#include "src/piyasa_kaldirac.h"
#include "src/piyasa_olay.h"
#include "src/piyasa_sim.h"
#include "src/kayit.h"

// BORSA_BASIZ: arayuz hic derlenmez (denge testi gibi penceresiz araclar icin).
#ifndef BORSA_BASIZ
#include "src/arayuz_temel.h"
#include "src/arayuz_oyun.h"
#include "src/arayuz_grafik.h"
#include "src/arayuz_pencere.h"
#include "src/arayuz_pencere2.h"
#include "src/arayuz_menu.h"
#include "src/uygulama.h"

// BORSA_ANA_YOK: main() tanimlanmaz (testler kendi main()'ini yazar).
#ifndef BORSA_ANA_YOK
int main(void) {
    Uygulama u;
    UygulamaBaslat(u);
#if BORSA_WEB
    // Tarayicida sonsuz dongu KURULAMAZ (sekme kilitlenir); her kareyi tarayici
    // cagirir. simulate_infinite_loop=1 oldugu icin main()'in yigini korunur,
    // yani `u` gecerliligini surdurur.
    emscripten_set_main_loop_arg([](void* p) { UygulamaKare(*(Uygulama*)p); }, &u, 0, 1);
#else
    while (!u.cikisIstendi) UygulamaKare(u);
    UygulamaKapat(u);
#endif
    return 0;
}
#endif // BORSA_ANA_YOK
#endif // BORSA_BASIZ
