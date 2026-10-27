// Borsa Patronu - service worker: kurulan uygulama (PWA) internetsiz de acilsin.
//
// Strateji "once ag": internet varsa her dosya sunucudan taze gelir ve onbellek
// guncellenir (yeni surum yayinlaninca oyuncu eski kodda takili KALMAZ);
// internet yoksa son indirilen kopya onbellekten verilir.
// Kayitlar localStorage'da durur, bu dosya onlara dokunmaz.
// Chrome uzantisinda ve itch.io'da kaydedilmez (bkz. kabuk.js).

var ONBELLEK = 'borsa-patronu';
var DOSYALAR = ['./', 'index.html', 'index.js', 'index.wasm', 'kabuk.js', 'favicon.ico', 'site.webmanifest'];

self.addEventListener('install', function (e) {
  e.waitUntil(caches.open(ONBELLEK).then(function (c) { return c.addAll(DOSYALAR); }));
  self.skipWaiting();
});

self.addEventListener('activate', function (e) {
  e.waitUntil(self.clients.claim());
});

self.addEventListener('fetch', function (e) {
  if (e.request.method !== 'GET') return;
  e.respondWith(
    fetch(e.request).then(function (yanit) {
      if (yanit.ok && new URL(e.request.url).origin === self.location.origin) {
        var kopya = yanit.clone();
        caches.open(ONBELLEK).then(function (c) { c.put(e.request, kopya); });
      }
      return yanit;
    }).catch(function () {
      return caches.match(e.request, { ignoreSearch: true });
    })
  );
});
