#include <iostream>
#include <cstdlib>
#include <ctime>

const int BOYUT = 500;

struct Matris {
    int deger[BOYUT][BOYUT];

    // Rastgele gri gürültü ver
    void degereGriSesVer() {
        srand((unsigned int)time(0));
        for (int i = 0; i < BOYUT; i++)
            for (int j = 0; j < BOYUT; j++)
                deger[i][j] = rand() % 256;
    }

    void yazdir() {
        for (int i = 0; i < BOYUT; i++) {
            for (int j = 0; j < BOYUT; j++) {
                std::cout << deger[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    void sobel(int piksel_satir, int piksel_sutun) {
        // Çerçeve piksellerinde 3x3 komşuluk dışarı taşar, işlem yapma
        if (piksel_satir < 1 || piksel_satir > BOYUT - 2 ||
            piksel_sutun < 1 || piksel_sutun > BOYUT - 2) return;

        // Komşuları bir diziye al
        int komsular[3][3];
        for (int satir = -1; satir <= 1; satir++) {
            for (int sutun = -1; sutun <= 1; sutun++) {
                komsular[satir + 1][sutun + 1] =
                    deger[piksel_satir + satir][piksel_sutun + sutun];
            }
        }

        // Komşuları yazdır (kontrol için)
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                std::cout << komsular[i][j] << " ";
            }
            std::cout << std::endl;
        }

        // yatay değişim (Gx) uygula
    }
};

int main() {
    static Matris m;   // static: stack overflow olmasın
    m.degereGriSesVer();
    m.sobel(10, 20);
    std::cout << "merkez dogrulama: " << m.deger[10][20] << std::endl;
    return 0;
}
