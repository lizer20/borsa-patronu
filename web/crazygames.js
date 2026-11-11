// Borsa Patronu - CrazyGames koprusu (yalnizca CrazyGames paketinde; build.py crazygames).
//
// Sira: kabuk.js (Module) -> CrazyGames SDK -> bu dosya. Bu dosya SDK'yi baslatir,
// Module.platform ve Module.depolama'yi kurar, SONRA oyunu (index.js) yukler:
// oyun acilirken kaydi okudugu icin depo hazir olmadan baslamamali.
// SDK yoksa ya da baslatilamazsa (cevrimdisi, reklam engelleyici, baska bir alan
// adi) oyun yine acilir; kayit localStorage'a duser.
// Oyunun C++ tarafi: src/platform.h ("OYUN PORTALI KOPRUSU").

(function () {
  var oyunuYukle = function () {
    var s = document.createElement('script');
    s.src = 'index.js';
    s.async = true;
    document.body.appendChild(s);
  };

  var sdk = window.CrazyGames && window.CrazyGames.SDK;
  if (!sdk) { oyunuYukle(); return; }

  var bitti = false;
  var devam = function () {
    if (bitti) return;
    bitti = true;
    oyunuYukle();
  };
  setTimeout(devam, 6000); // SDK cevap vermezse oyuncu beklemesin

  sdk.init().then(function () {
    if (sdk.environment === 'disabled') return;
    var oyun = sdk.game;
    var platform = {
      oynaniyor: function (evet) {
        if (evet) oyun.gameplayStart(); else oyun.gameplayStop();
      },
      mutlu: function () { oyun.happytime(); },
      sessiz: !!(oyun.settings && oyun.settings.muteAudio),
      dil: (sdk.user && sdk.user.systemInfo && sdk.user.systemInfo.locale) || ''
    };
    oyun.addSettingsChangeListener(function (ayar) {
      platform.sessiz = !!ayar.muteAudio;
    });
    Module.platform = platform;
    // Kayit: oyuncu girisliyse hesabina esitlenir (gelistirici portalinda
    // "Progress Save" secili olmali; degilse modul kapalidir ve localStorage kalir).
    var veri = sdk.data;
    Module.depolama = {
      getItem: function (k) {
        try { return veri.getItem(k); } catch (e) { return localStorage.getItem(k); }
      },
      setItem: function (k, v) {
        try { veri.setItem(k, v); } catch (e) { localStorage.setItem(k, v); }
      }
    };
    oyun.loadingStart();
    var eski = Module.onRuntimeInitialized;
    Module.onRuntimeInitialized = function () {
      if (eski) eski();
      oyun.loadingStop();
    };
  }).catch(function () {}).then(devam);
})();
