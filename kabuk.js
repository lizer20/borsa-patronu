// Borsa Patronu - web sayfasinin kucuk yardimci betigi.
//
// Bu kod eskiden kabuk.html icinde satir ici <script> olarak duruyordu. Chrome
// uzantilarinin (Chrome Web Store) guvenlik kurali (CSP) satir ici betige ve
// HTML icindeki "oncontextmenu=..." gibi olay ozniteliklerine izin vermez; bu
// yuzden ayri bir dosyada. Ayni dosya hem web sitesinde hem uzantida kullanilir.
// index.js'den ONCE yuklenmelidir (Module nesnesini o okur).

var Module = {
  canvas: document.getElementById('canvas'),
  setStatus: function (metin) {
    if (!metin) document.getElementById('durum').style.display = 'none';
  },
  onRuntimeInitialized: function () {
    document.getElementById('durum').style.display = 'none';
  },
  onAbort: function (neden) {
    document.getElementById('durum').style.display = 'none';
    document.getElementById('hata').textContent =
      'Oyun baslatilamadi / The game could not start.\n' + (neden || '');
  }
};

// Sag tik menusu oyunun ustunde acilmasin.
Module.canvas.addEventListener('contextmenu', function (e) { e.preventDefault(); });

// TEST MODU: adrese "?zamanlayici" eklenirse kareler requestAnimationFrame
// yerine zamanlayiciyla uretilir. Tarayici, GORUNMEYEN bir sayfada
// requestAnimationFrame'i hic cagirmaz; otomatik testler (gizli sekme/panel)
// oyunu ancak bu modda ilerletebilir. Normal oyuncuyu etkilemez.
if (location.search.indexOf('zamanlayici') >= 0) {
  window.requestAnimationFrame = function (f) {
    return setTimeout(function () { f(performance.now()); }, 16);
  };
  window.cancelAnimationFrame = function (id) { clearTimeout(id); };
}

// Klavye kisayollari (Bosluk, 1-8, M) icin tuvalin odakta olmasi gerekir;
// iframe icinde ilk tiklamada odagi tuvale ver. Bosluk tusu sayfayi kaydirmasin.
window.addEventListener('mousedown', function () {
  window.focus();
  Module.canvas.focus();
});
window.addEventListener('keydown', function (e) {
  if (e.code === 'Space') e.preventDefault();
});

// Kurulabilir uygulama (PWA): internetsiz acilis icin service worker (sw.js).
// Yalnizca web sitesinde: Chrome uzantisinda (chrome-extension:) ve bir
// iframe'in icinde (itch.io) kaydedilmez.
if ('serviceWorker' in navigator && /^https?:$/.test(location.protocol) && window.self === window.top) {
  window.addEventListener('load', function () {
    navigator.serviceWorker.register('sw.js').catch(function () {});
  });
}
