// Borsa Patronu - Chrome uzantisinin arka plan betigi.
// Uzantinin tek isi oyunu acmak: arac cubugundaki simgeye tiklayinca oyun yeni
// bir sekmede acilir. Ilk kurulumda da bir kez acilir ki oyuncu nerede oldugunu
// gorsun. Hicbir izin istenmez, hicbir sayfaya erisilmez, disariya veri gitmez.
chrome.action.onClicked.addListener(function () {
  chrome.tabs.create({ url: 'index.html' });
});

chrome.runtime.onInstalled.addListener(function (ayrinti) {
  if (ayrinti.reason === 'install') chrome.tabs.create({ url: 'index.html' });
});
