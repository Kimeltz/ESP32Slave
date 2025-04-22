#ifndef SignalProcessing_h
#define SignalProcessing_h

/**
 * @brief Class untuk menghitung moving average (rata-rata bergerak).
 * 
 * Class ini menyimpan data dalam buffer dan menghitung rata-rata bergerak
 * berdasarkan data yang diterima.
 */
class movingAverage
{
    public:
         /**
         * @brief Konstruktor untuk class movingAverage.
         * 
         * Konstruktor ini akan menginisialisasi ukuran buffer, indeks, jumlah
         * data yang diterima, dan buffer itu sendiri.
         * 
         * @param bufferSize Ukuran dari buffer yang digunakan untuk menyimpan data.
         */
        movingAverage(int bufferSize);

         /**
         * @brief Destruktor untuk class movingAverage.
         * 
         * Fungsi ini digunakan untuk membebaskan memori yang digunakan oleh buffer
         * ketika objek dihancurkan.
         */
        ~movingAverage();

         /**
         * @brief Menginisialisasi buffer dan memori.
         * 
         * @return int Status inisialisasi, 0 jika berhasil, -1 jika gagal.
         */
        bool init();

         /**
         * @brief Mengupdate nilai rata-rata bergerak dengan data baru.
         * 
         * Fungsi ini menghitung rata-rata bergerak setelah menerima data baru.
         * Jika jumlah data sudah mencapai ukuran buffer, maka data yang lama
         * akan digantikan dengan yang baru.
         * 
         * @param newData Data baru yang akan dimasukkan ke dalam buffer.
         * @return float Nilai rata-rata bergerak setelah data baru dimasukkan.
         */
        float update(float newData);

         /**
         * @brief Mengembalikan pointer ke buffer yang digunakan untuk menyimpan data.
         * 
         * @return float* Pointer ke buffer.
         */
        float* getBuffer();

         /**
         * @brief Mengembalikan nilai rata-rata bergerak saat ini.
         * 
         * @return float Nilai rata-rata bergerak.
         */
        float getValue();

         /**
         * @brief Mengembalikan ukuran buffer yang digunakan untuk menyimpan data.
         * 
         * @return int Ukuran buffer.
         */
        int getSize();

         /**
         * @brief Mengembalikan jumlah data yang sudah diterima dan diproses.
         * 
         * @return int Jumlah data yang sudah diterima.
         */
        int getCount();

    private:
        int _size; 
        float* _buffer;
        int _index;
        int _count;
        float _sum;

};

float roundToNearest005(float val);
float* roundToNearest005(float* val, int size);

#endif