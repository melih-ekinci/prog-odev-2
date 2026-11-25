// Programlamaya Giris - Odev 2
// 500x500 gri matris uzerinde Sobel, Prewitt, Canny, Harris ve Histogram
// Dis kutuphane kullanilmamistir, tum algoritmalar sifirdan yazilmistir.

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

struct Matris {
    int satir_sayisi;
    int sutun_sayisi;
    std::vector<std::vector<int> > deger;   // deger[i][j] olarak kullanilir

    Matris(int satir, int sutun)
        : satir_sayisi(satir), sutun_sayisi(sutun),
          deger(satir, std::vector<int>(sutun, 0)) {}

    // Rastgele gri gürültü ver
    void degereGriSesVer() {
        for (int i = 0; i < satir_sayisi; i++)
            for (int j = 0; j < sutun_sayisi; j++)
                deger[i][j] = rand() % 256;
    }

    void yazdir() const {
        for (int i = 0; i < satir_sayisi; i++) {
            for (int j = 0; j < sutun_sayisi; j++)
                std::cout << deger[i][j] << " ";
            std::cout << std::endl;
        }
    }

    // (satir, sutun) merkezli 3x3 komşuluğu yeni bir Matris olarak döndürür
    Matris komsuluk(int satir, int sutun) const {
        Matris komsular(3, 3);
        for (int a = -1; a <= 1; a++)
            for (int b = -1; b <= 1; b++)
                komsular.deger[a + 1][b + 1] = deger[satir + a][sutun + b];
        return komsular;
    }

    // Sobel büyüklüğü: çerçevede 0 döner
    int sobel(int satir, int sutun) const {
        if (satir < 1 || satir > satir_sayisi - 2 ||
            sutun < 1 || sutun > sutun_sayisi - 2) return 0;

        Matris k = komsuluk(satir, sutun);

        int Gx = 0, Gy = 0;
        for (int s = 0; s < 3; s++) {
            for (int c = 0; c < 3; c++) {
                int x = k.deger[s][c] * (c - 1);   // yatay: -1, 0, +1
                if (s == 1) x *= 2;                // orta satır 2 katı
                Gx += x;

                int y = k.deger[s][c] * (s - 1);   // dikey: -1, 0, +1
                if (c == 1) y *= 2;                // orta sütun 2 katı
                Gy += y;
            }
        }
        int M = (int)std::sqrt((double)(Gx * Gx + Gy * Gy));
        if (M > 255) M = 255;
        return M;
    }

    // Prewitt büyüklüğü: Sobel ile aynı, ortadaki satır/sütun 2 katı alınmaz
    int prewitt(int satir, int sutun) const {
        if (satir < 1 || satir > satir_sayisi - 2 ||
            sutun < 1 || sutun > sutun_sayisi - 2) return 0;

        Matris k = komsuluk(satir, sutun);

        int Gx = 0, Gy = 0;
        for (int s = 0; s < 3; s++) {
            for (int c = 0; c < 3; c++) {
                Gx += k.deger[s][c] * (c - 1);   // yatay: -1, 0, +1
                Gy += k.deger[s][c] * (s - 1);   // dikey: -1, 0, +1
            }
        }
        int M = (int)std::sqrt((double)(Gx * Gx + Gy * Gy));
        if (M > 255) M = 255;
        return M;
    }

    // Sobel gradyanı (işaretli, kırpılmamış): gx ve gy matrislerini doldurur
    void gradyan(Matris& gx, Matris& gy) const {
        gx = Matris(satir_sayisi, sutun_sayisi);
        gy = Matris(satir_sayisi, sutun_sayisi);
        for (int i = 1; i < satir_sayisi - 1; i++) {
            for (int j = 1; j < sutun_sayisi - 1; j++) {
                Matris k = komsuluk(i, j);
                int x = 0, y = 0;
                for (int s = 0; s < 3; s++) {
                    for (int c = 0; c < 3; c++) {
                        int a = k.deger[s][c] * (c - 1);
                        if (s == 1) a *= 2;
                        x += a;
                        int b = k.deger[s][c] * (s - 1);
                        if (c == 1) b *= 2;
                        y += b;
                    }
                }
                gx.deger[i][j] = x;
                gy.deger[i][j] = y;
            }
        }
    }

    // CANNY 1. adım: 5x5 Gaussian bulanıklaştırma (kenarlar orijinal kalır)
    Matris gaussian() const {
        static const int kernel[5][5] = {
            { 2,  4,  5,  4, 2},
            { 4,  9, 12,  9, 4},
            { 5, 12, 15, 12, 5},
            { 4,  9, 12,  9, 4},
            { 2,  4,  5,  4, 2}
        };
        const int toplamAgirlik = 159;

        Matris sonuc = *this;   // çerçeve (2 piksel) olduğu gibi kopyalanır
        for (int i = 2; i < satir_sayisi - 2; i++) {
            for (int j = 2; j < sutun_sayisi - 2; j++) {
                int toplam = 0;
                for (int a = -2; a <= 2; a++)
                    for (int b = -2; b <= 2; b++)
                        toplam += deger[i + a][j + b] * kernel[a + 2][b + 2];
                sonuc.deger[i][j] = toplam / toplamAgirlik;
            }
        }
        return sonuc;
    }

    // CANNY: kenar pikseli 255, değilse 0
    Matris canny(int dusuk_esik = 30, int yuksek_esik = 80) const {
        const double PI = 3.14159265358979;

        // 1) Gaussian bulanıklaştırma
        Matris bulanik = gaussian();

        // 2) Sobel gradyan büyüklüğü
        Matris gx(1, 1), gy(1, 1);
        bulanik.gradyan(gx, gy);
        Matris buyukluk(satir_sayisi, sutun_sayisi);
        for (int i = 0; i < satir_sayisi; i++)
            for (int j = 0; j < sutun_sayisi; j++)
                buyukluk.deger[i][j] = (int)std::sqrt(
                    (double)(gx.deger[i][j] * gx.deger[i][j] +
                             gy.deger[i][j] * gy.deger[i][j]));

        // 3) Non-Maximum Suppression: gradyan yönündeki iki komşudan
        //    küçük olan pikseller silinir
        Matris nms(satir_sayisi, sutun_sayisi);
        for (int i = 2; i < satir_sayisi - 2; i++) {
            for (int j = 2; j < sutun_sayisi - 2; j++) {
                double aci = std::atan2((double)gy.deger[i][j],
                                        (double)gx.deger[i][j]) * 180.0 / PI;
                if (aci < 0) aci += 180.0;   // 0..180 arası

                int di, dj;   // gradyan yönündeki komşunun kayması
                if (aci < 22.5 || aci >= 157.5) { di = 0;  dj = 1;  }  // 0 derece
                else if (aci < 67.5)            { di = 1;  dj = 1;  }  // 45 derece
                else if (aci < 112.5)           { di = 1;  dj = 0;  }  // 90 derece
                else                            { di = 1;  dj = -1; }  // 135 derece

                int m = buyukluk.deger[i][j];
                if (m >= buyukluk.deger[i + di][j + dj] &&
                    m >= buyukluk.deger[i - di][j - dj])
                    nms.deger[i][j] = m;
            }
        }

        // 4) Çift eşik: 255 = güçlü, 128 = zayıf, 0 = silinmiş
        Matris sonuc(satir_sayisi, sutun_sayisi);
        for (int i = 0; i < satir_sayisi; i++) {
            for (int j = 0; j < sutun_sayisi; j++) {
                int m = nms.deger[i][j];
                if (m >= yuksek_esik)      sonuc.deger[i][j] = 255;
                else if (m >= dusuk_esik)  sonuc.deger[i][j] = 128;
            }
        }

        // Hysteresis: güçlü komşusu olan zayıf piksel güçlü olur.
        // Zincirleme yayılsın diye değişiklik kalmayana kadar tekrarla.
        bool degisti = true;
        while (degisti) {
            degisti = false;
            for (int i = 1; i < satir_sayisi - 1; i++) {
                for (int j = 1; j < sutun_sayisi - 1; j++) {
                    if (sonuc.deger[i][j] != 128) continue;
                    bool guclu_var = false;
                    for (int a = -1; a <= 1 && !guclu_var; a++)
                        for (int b = -1; b <= 1; b++)
                            if (sonuc.deger[i + a][j + b] == 255) { guclu_var = true; break; }
                    if (guclu_var) { sonuc.deger[i][j] = 255; degisti = true; }
                }
            }
        }

        // Hâlâ zayıf kalanlar silinir
        for (int i = 0; i < satir_sayisi; i++)
            for (int j = 0; j < sutun_sayisi; j++)
                if (sonuc.deger[i][j] != 255) sonuc.deger[i][j] = 0;
        return sonuc;
    }

    // HARRIS: köşe pikseli 255, değilse 0
    // esik_orani: eşik = esik_orani * (en büyük R değeri)
    Matris harris(double k = 0.04, double esik_orani = 0.01) const {
        Matris gx(1, 1), gy(1, 1);
        gradyan(gx, gy);

        // Her piksel için 3x3 penceredeki Ixx, Iyy, Ixy toplamlarından R hesapla
        std::vector<std::vector<double> > R(satir_sayisi,
                                            std::vector<double>(sutun_sayisi, 0.0));
        double enBuyukR = 0.0;
        for (int i = 2; i < satir_sayisi - 2; i++) {
            for (int j = 2; j < sutun_sayisi - 2; j++) {
                double Ixx = 0, Iyy = 0, Ixy = 0;
                for (int a = -1; a <= 1; a++) {
                    for (int b = -1; b <= 1; b++) {
                        double x = gx.deger[i + a][j + b];
                        double y = gy.deger[i + a][j + b];
                        Ixx += x * x;
                        Iyy += y * y;
                        Ixy += x * y;
                    }
                }
                double det   = Ixx * Iyy - Ixy * Ixy;
                double trace = Ixx + Iyy;
                R[i][j] = det - k * trace * trace;
                if (R[i][j] > enBuyukR) enBuyukR = R[i][j];
            }
        }

        // R > eşik ise köşe
        double esik = esik_orani * enBuyukR;
        Matris sonuc(satir_sayisi, sutun_sayisi);
        for (int i = 0; i < satir_sayisi; i++)
            for (int j = 0; j < sutun_sayisi; j++)
                if (R[i][j] > esik) sonuc.deger[i][j] = 255;
        return sonuc;
    }

    // Tüm resme Sobel uygula (çerçeve 0 kalır)
    Matris sobelResim() const {
        Matris sonuc(satir_sayisi, sutun_sayisi);
        for (int i = 0; i < satir_sayisi; i++)
            for (int j = 0; j < sutun_sayisi; j++)
                sonuc.deger[i][j] = sobel(i, j);
        return sonuc;
    }

    // Tüm resme Prewitt uygula (çerçeve 0 kalır)
    Matris prewittResim() const {
        Matris sonuc(satir_sayisi, sutun_sayisi);
        for (int i = 0; i < satir_sayisi; i++)
            for (int j = 0; j < sutun_sayisi; j++)
                sonuc.deger[i][j] = prewitt(i, j);
        return sonuc;
    }

    // HISTOGRAM: hist[g] = g gri değerinin kaç piksel olduğu
    std::vector<int> histogram() const {
        std::vector<int> hist(256, 0);
        for (int i = 0; i < satir_sayisi; i++)
            for (int j = 0; j < sutun_sayisi; j++)
                hist[deger[i][j]]++;
        return hist;
    }

    // Matrisi .pgm (P2, düz metin) dosyası olarak kaydeder
    bool pgmKaydet(const std::string& dosya_adi) const {
        std::ofstream f(dosya_adi.c_str());
        if (!f) return false;
        f << "P2\n" << sutun_sayisi << " " << satir_sayisi << "\n255\n";
        for (int i = 0; i < satir_sayisi; i++) {
            for (int j = 0; j < sutun_sayisi; j++)
                f << deger[i][j] << " ";
            f << "\n";
        }
        return true;
    }
};

// Histogramı histogram.txt'ye yazar: 1. sütun gri değer, 2. sütun adet
bool histogramDosyayaYaz(const std::vector<int>& hist, const std::string& dosya_adi) {
    std::ofstream f(dosya_adi.c_str());
    if (!f) return false;
    for (int g = 0; g < 256; g++)
        f << g << "\t" << hist[g] << "\n";
    return true;
}

// Konsola 16'lı gruplar halinde ASCII çubuk grafik çizer
void histogramKonsolaCiz(const std::vector<int>& hist) {
    int grup[16];
    int enBuyuk = 0;
    for (int g = 0; g < 16; g++) {
        grup[g] = 0;
        for (int k = 0; k < 16; k++) grup[g] += hist[g * 16 + k];
        if (grup[g] > enBuyuk) enBuyuk = grup[g];
    }
    const int MAKS_UZUNLUK = 50;   // en uzun çubuk 50 karakter
    for (int g = 0; g < 16; g++) {
        int uzunluk = (enBuyuk > 0) ? grup[g] * MAKS_UZUNLUK / enBuyuk : 0;
        std::cout << std::setw(3) << g * 16 << "-" << std::setw(3) << g * 16 + 15 << " | ";
        for (int u = 0; u < uzunluk; u++) std::cout << '#';
        std::cout << " " << grup[g] << std::endl;
    }
}

// Sıfır olmayan piksel sayısı
int sifirDegilSay(const Matris& m) {
    int n = 0;
    for (int i = 0; i < m.satir_sayisi; i++)
        for (int j = 0; j < m.sutun_sayisi; j++)
            if (m.deger[i][j] != 0) n++;
    return n;
}

int main() {
    const int ROWS = 500, COLS = 500;
    srand((unsigned int)time(0));

    // 1) 500x500 rastgele gri matris
    Matris orijinal(ROWS, COLS);
    orijinal.degereGriSesVer();
    std::cout << "500x500 rastgele gri matris olusturuldu." << std::endl;

    // 2) Algoritmalar
    Matris sobelCikis   = orijinal.sobelResim();
    Matris prewittCikis = orijinal.prewittResim();
    Matris cannyCikis   = orijinal.canny(30, 80);
    Matris harrisCikis  = orijinal.harris(0.04, 0.2);

    std::cout << "Sobel   sifir olmayan piksel: " << sifirDegilSay(sobelCikis)   << std::endl;
    std::cout << "Prewitt sifir olmayan piksel: " << sifirDegilSay(prewittCikis) << std::endl;
    std::cout << "Canny   kenar pikseli:        " << sifirDegilSay(cannyCikis)   << std::endl;
    std::cout << "Harris  kose pikseli:         " << sifirDegilSay(harrisCikis)  << std::endl;

    // 3) Histogram
    std::vector<int> hist = orijinal.histogram();
    std::cout << "\nHistogram (16'li gruplar):" << std::endl;
    histogramKonsolaCiz(hist);

    // 4) Dosyalara kaydet
    bool tamam = true;
    tamam = orijinal.pgmKaydet("orijinal.pgm")     && tamam;
    tamam = sobelCikis.pgmKaydet("sobel.pgm")      && tamam;
    tamam = prewittCikis.pgmKaydet("prewitt.pgm")  && tamam;
    tamam = cannyCikis.pgmKaydet("canny.pgm")      && tamam;
    tamam = harrisCikis.pgmKaydet("harris.pgm")    && tamam;
    tamam = histogramDosyayaYaz(hist, "histogram.txt") && tamam;

    if (tamam)
        std::cout << "\nDosyalar kaydedildi: orijinal.pgm, sobel.pgm, prewitt.pgm, "
                     "canny.pgm, harris.pgm, histogram.txt" << std::endl;
    else
        std::cout << "\nUYARI: bazi dosyalar yazilamadi!" << std::endl;

    return 0;
}
