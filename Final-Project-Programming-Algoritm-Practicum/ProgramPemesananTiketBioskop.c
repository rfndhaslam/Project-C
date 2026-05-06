#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// deklarasi variabel global
int menu, pesantahun, x, p;
int banyak, i, j, n, genree;
char uname[100], pass[100], genre[100], pesanjudul[2000], metode[2000], smen[100];

// deklarasi variabel struct
struct film
{
    char judul[100], deskripsi[1000];
    int available, tahun, harga, genre;
} k, temp, t[100], s;

struct user
{
    char nama[100], password[100], judulfilm[1000], deskripsifilm[1000];
    int tahunfilm, genrefilm, hargatiket, bayar, ATM;
} user, uus[100];

// deklarasi file
FILE *f_datauser;
FILE *f_listfilm;
FILE *f_bioskop;
FILE *f_bioskopterjual;

// deklarasi fungsi void
void daftarfilm();
void menuadmin();
void regis();
void menuuser();
void userlogin();

void daftarfilm()
{
    f_listfilm = fopen("Listfilm.dat", "rb");
    i = 1;
    printf("=====Daftar List Film=====\n");
    while (fread(&k, sizeof(k), 1, f_listfilm) == 1)
    {
        printf("\n=====Film ke-%d=====\n", i);
        if (k.genre != 1 && k.genre != 2 && k.genre != 3 && k.genre != 4 && k.genre != 5) // sarching binary
        {
            strcpy(genre, "Genre Film tidak di ketahui");
        }
        if (k.genre == 1) // searching
        {
            strcpy(genre, "Action");
        }
        if (k.genre == 2) // searching
        {
            strcpy(genre, "Horror");
        }
        if (k.genre == 3)
        {
            strcpy(genre, "Romance");
        }
        if (k.genre == 4)
        {
            strcpy(genre, "Thriller");
        }
        if (k.genre == 5)
        {
            strcpy(genre, "Drama");
        }
        printf("Genre film          : %s\n", genre);
        printf("Judul film          : %s\n", k.judul);
        printf("Harga               : %d\n", k.harga);
        printf("Tahun Produksi Film : %d\n", k.tahun);
        printf("Deskripsi           : %s\n", k.deskripsi);
        i++;
    }

    fclose(f_listfilm);
}

void menuadmin()
{
    system("cls");
    printf("\n====== Masukan Username Dan Password Anda =======");
    printf("\n\n Username admin : ");
    gets(uname);
    printf(" Password admin : ");
    gets(pass);
    system("cls");
    if (strcmp(uname, "admin") == 0 && strcmp(pass, "66") == 0)
    {
        printf("\n\n\n========SELAMAT DATANG ADMIN========\n\n\n.");
        system("pause");
        system("cls");
        printf("========Menu Admin========\n");
        printf("1. Lihat film yang tersedia\n");
        printf("2. Masukkan judul film terbaru\n");
        printf("3. Pembelian Terbaru User\n");
        printf("4. Hapus semua daftar tiket film yang dijual\n");
        printf("5. Kembali\n\n");
        printf("Masukan pilihan menu yang tersedia :");
        scanf("%d", &menu);
        getchar();
        switch (menu)
        {
        case 1:
        {
            system("cls");
            daftarfilm();
            system("pause");
            menuadmin(); // rekursif
            break;
        }
        case 2:
        {
            system("cls");
            printf("=====Tiket Film Terbaru yang Ingin Dijual=====\n\n");
            f_listfilm = fopen("Listfilm.dat", "ab+");

            printf(" Silahkan masukkan jumlah film : ");
            scanf("%d", &n);
            getchar();
            for (i = 1; i <= n; i++)
            {
                system("cls");
                printf("=====Tiket Film ke-%d=====\n", i);
                printf("Genre Film :\n");
                printf("1. Action\n");
                printf("2. Horror\n");
                printf("3. Romance\n");
                printf("4. Thriller\n");
                printf("5. Drama\n");
                printf("Genre Film : ");
                scanf("%d", &k.genre);
                getchar();
                system("cls");
                printf("=====Film ke-%d=====\n", i);
                printf("Judul Film : ");
                gets(k.judul);
                system("cls");
                if (k.genre != 1 && k.genre != 2 && k.genre != 3 && k.genre != 4 && k.genre != 5)
                {
                    strcpy(genre, "Genre film tidak diketahui");
                }
                if (k.genre == 1)
                {
                    strcpy(genre, "Action");
                }
                if (k.genre == 2)
                {
                    strcpy(genre, "Horror");
                }
                if (k.genre == 3)
                {
                    strcpy(genre, "Romance");
                }
                if (k.genre == 4)
                {
                    strcpy(genre, "Thriller");
                }
                if (k.genre == 5)
                {
                    strcpy(genre, "Drama");
                }
                x = 1;
                printf("Jumlah tiket film ber genre %s :", genre);
                scanf("%d", &banyak);
                getchar();
                for (j = 1; j <= banyak; j++)
                {
                    system("cls");
                    printf("=====Film %s ke-%d=====\n", k.judul, x);
                    printf("Masukan Tahun Produksi Film         : ");
                    scanf("%d", &k.tahun);
                    getchar();
                    printf("Masukan Deskripsi film terkait      : ");
                    gets(k.deskripsi);
                    printf("Harga                               : ");
                    scanf("%d", &k.harga);
                    getchar();
                    fwrite(&k, sizeof(k), 1, f_listfilm);
                    x++;
                }
                getchar();
            }
            fclose(f_listfilm);
            printf("\nData telah di tambahkan...\n");
            system("pause");
            menuadmin(); // rekursif
            break;
        }
        case 3:
        {
            system("cls");
            f_datauser = fopen("User.dat", "rb");
            while (fread(&user, sizeof(user), 1, f_datauser) == 1)
            {
                printf("\n=====Tiket Film Terakhir Dibeli User=====\n\n");
                if (k.genre != 1 && k.genre != 2 && k.genre != 3 && k.genre != 4 && k.genre != 5)
                {
                    strcpy(genre, "Genre film tidak di ketahui");
                }
                if (user.genrefilm == 1)
                {
                    strcpy(genre, "Action");
                }
                if (user.genrefilm == 2)
                {
                    strcpy(genre, "Horror");
                }
                if (user.genrefilm == 3)
                {
                    strcpy(genre, "Romance");
                }
                if (user.genrefilm == 4)
                {
                    strcpy(genre, "Thriller");
                }
                if (user.genrefilm == 5)
                {
                    strcpy(genre, "Drama");
                }
                printf("Nama Pembeli                 : %s\n", user.nama);
                printf("Genre Film                   : %s\n", genre);
                printf("Judul Film                   : %s\n", user.judulfilm);
                printf("Harga                        : %d\n", user.hargatiket);
                printf("Masukkan Tahun Produksi Film : %d\n", user.tahunfilm);
                if (user.bayar == 1)
                {
                    if (user.ATM == 1)
                    {
                        strcpy(metode, "ATM (BCA)");
                    }
                    if (user.ATM == 2)
                    {
                        strcpy(metode, "ATM (Mandiri)");
                    }
                    if (user.ATM == 3)
                    {
                        strcpy(metode, "ATM (BRI)");
                    }
                    if (user.ATM == 4)
                    {
                        strcpy(metode, "ATM (BNI)");
                    }
                }
                else
                {
                    strcpy(metode, "Transaksi Gagal");
                }
                printf("Metode Pembayaran	     : %s\n\n", metode);
            }
            fclose(f_datauser);
            system("pause");
            menuadmin();
            break;
        }
        case 4:
        {
            system("cls");
            remove("Listfilm.dat");
            f_datauser = fopen("Listfilm.dat", "wb");
            fwrite(&temp, sizeof(temp), 1, f_listfilm);
            fclose(f_datauser);
            printf("\n\nTekan Enter untuk kembali");
            system("pause");
            menuadmin(); // rekursif
            break;
        }
        case 5:
        {
            system("cls");
            main(); // pergi ke main
            break;
        }
        default:
        {
            printf("Pilihan Anda Salah\nSilahkan Coba Lagi\n\n");
            system("pause");
            system("cls");
            menuadmin(); // rekursif
            break;
        }
        }
    }
    else
    {
        menuadmin(); // rekursif
    }
}

void regis()
{
    int temu = 0;
    char usname[100], passw[100];
    FILE *f_datauser;
    system("cls");
    printf("==============MENU REGISTRASI USER==============\n");
    printf("=======SILAHKAN ISI USERNAME DAN PASSWORD=======\n\n");
    printf(" Masukan Username : ");
    gets(usname);
    printf(" Masukan Password : ");
    gets(passw);
    f_datauser = fopen("User.dat", "rb");
    temu = 0;
    while (fread(&user, sizeof(user), 1, f_datauser) == 1)
    {
        if (strcmp(user.nama, usname) == 0)
        {
            temu = 1;
        }
    }
    fclose(f_datauser);
    if (temu == 0)
    {
        f_datauser = fopen("User.dat", "ab+");
        strcpy(user.nama, usname);
        strcpy(user.password, passw);
        fwrite(&user, sizeof(user), 1, f_datauser);
        fclose(f_datauser);
        printf("\n\n\t\t REGISTRASI BERHASIL! :)\n\n\n");
        getchar();
        system("pause");
        system("cls");
        main(); // pergi ke main
    }
    else
    {
        system("cls");
        printf("\n\n\t\tUsername Telah Digunakan, silahkan coba lagi dangan username lain\n\n");
        system("pause");
        regis(); // rekursif
    }
}

void menuuser()
{
    system("cls");
    printf("====MENU USER====\n");
    printf("1. Lihat daftar tiket film yang tersedia\n");
    printf("2. Pesan tiket\n");
    printf("3. Pembelian Terakhir\n");
    printf("4. Kembali\n");
    printf("Masukkan Pilihan: ");
    scanf("%d", &menu);
    getchar();
    switch (menu)
    {
    case 1:
    {
        system("cls");
        daftarfilm(); // pergi ke fungsi void daftarkendaraan
        system("pause");
        menuuser(); // rekursif
        break;
    case 2:
    {
        system("cls");
        printf("*GENRE FILM*\n");
        printf("1. Action\n");
        printf("2. Horror\n");
        printf("3. Romance\n");
        printf("4. Thriller\n");
        printf("5. Drama\n");
        printf("Masukkan genre film yang ingin anda tonton : ");
        scanf("%d", &genree);
        getchar();
        i = 0;
        f_listfilm = fopen("Listfilm.dat", "rb");
        while (fread(&k, sizeof(k), 1, f_listfilm) == 1)
        {
            if(genree==k.genre) // searching binary
            {
                t[i]=k;
                i++;
            }
        }
        fclose(f_listfilm);
        system("cls");
        printf("\n\n=====Berikut adalah daftar film berdasarkan genre yang anda pilih=====\n\n");
        for (j=0;j<=i-1;j++)
        {
            printf(" Film %d\n", j+1);
            printf(" Judul film     :%s\n", t[j].judul);
            printf(" Tahun produksi :%d\n", t[j].tahun);
            printf(" Harga          :%d\n", t[j].harga);
            printf(" Deskripsi film :%s\n\n", t[j].deskripsi);
        }
        printf(" Masukan judul film tiket yang ingin dibeli    : ");
        gets(pesanjudul);
        printf(" Masukan tahun produksi film yang ingin beli   : ");
        scanf("%d", &pesantahun);
        getchar();
        j=0;

        while((j<=i-1) && (t[j].tahun != pesantahun) && strcmp(t[j].judul, pesanjudul) == 0)
        {
            j=j+1;
        }
        for(p=0;p<=j;p++)
        {
            if (t[j].tahun==pesantahun)
            {
                f_datauser = fopen("User.dat", "rb+");
                while (fread(&user, sizeof(user), 1, f_datauser) == 1)
                {
                    if (strcmp(uname, user.nama) == 0 && strcmp(pass, user.nama) == 0) // searching
                    {
                        fseek(f_datauser, -sizeof(user), SEEK_CUR);
                        strcpy(user.judulfilm, t[j].judul);
                        strcpy(user.deskripsifilm, t[j].deskripsi);
                        user.hargatiket = t[j].harga;
                        user.tahunfilm = t[j].tahun;
                        user.genrefilm = t[j].genre;
                        // bagian pilih metode pembayaran
                        system("cls");
                        printf("Pilih Metode Pembayaran : \n");
                        printf("1. ATM\n");
                        printf("Pilih Metode Pembayaran : ");
                        scanf("%d", &user.bayar);
                        getchar();
                        if (user.bayar == 1)
                        {
                            system("cls");
                            printf("Pilih atm yang telah bermitra dengan kami :\n");
                            printf("1. BCA\n");
                            printf("2. Mandiri\n");
                            printf("3. BRI\n");
                            printf("4. BNI\n");
                            printf("Masukan pilihan yang anda inginkan : ");
                            scanf("%d", &user.ATM);
                            getchar();
                            switch (user.ATM)
                            {
                            case 1:
                            {
                                printf("\nSilahkan transfer sejumlah Rp.%d dengan nomer rekening 09876543234567890 ke Bank BCA atas nama D'TICS\nBawa bukti transfer saat mengambil tiket\n\n", user.hargatiket);
                                break;
                            }
                            case 2:
                            {
                                printf("\nSilahkan transfer sejumlah Rp.%d dengan nomer rekening 12345678987654321 ke Bank Mandiri atas nama D'TICS\nBawa bukti transfer saat mengambil tiket\n\n", user.hargatiket);
                                break;
                            }
                            case 3:
                            {
                                printf("\nSilahkan transfer sejumlah Rp.%d dengan nomer rekening 00998877665544332 ke Bank BRI atas nama D'TICS\nBawa bukti transfer saat mengambil tiket\n\n", user.hargatiket);
                                break;
                            }
                            case 4:
                            {
                                printf("\nSilahkan transfer sejumlah Rp.%d dengan nomer rekening 11223344556677889 ke Bank BNI atas nama D'TICS\nBawa bukti transfer saat mengambil tiket\n\n", user.hargatiket);
                                break;
                            }
                            default:
                            {
                                break;
                            }
                            }
                        }
                        else
                        {
                        	system("cls");
                            printf("\nPILIHAN TIDAK ADA\n");
                            printf("PEMESANAN GAGAL\n\n");
                            system("pause");
                            menuuser();
                        }
                        system("pause");
                        fwrite(&user, sizeof(user), 1, f_datauser);
                        fclose(f_datauser);
                        f_listfilm = fopen("Listfilm.dat", "rb");
                        f_bioskop = fopen("Bioskop.dat", "wb");
                        while (fread(&k, sizeof(k), 1, f_listfilm == 1))
                        {
                            if (strcmp(k.judul, pesanjudul) != 0 || k.tahun != pesantahun) // searching binary
                            {
                                fwrite(&k, sizeof(k), 1, f_bioskop);
                            }
                        }
                        fclose(f_listfilm);
                        fclose(f_bioskop);
                        break;
                    }
                }
            }
        }
        system("cls");
        printf("\n=====Pemesanan Berhasil!=====\n");
        system("pause");
        menuuser(); // rekursif
        break;
    }
    case 3:
    {
         system("cls");
        printf("=====Daftar Tiket Film Yang Terakhir Dibeli=====");
        FILE *f_datauser;
        f_datauser = fopen("User.dat", "rb");
        i = 1;
        while (fread(&user, sizeof(user), 1, f_datauser) == 1)
        {
            if (strcmp(uname, user.nama) == 0) // searching binary
            {
                printf("\n\n=====Tiket Film Terakhir Dibeli=====\n\n");
                if (user.genrefilm != 1 && user.genrefilm != 2 && user.genrefilm != 3 && user.genrefilm != 4 && user.genrefilm != 5)
                {
                    strcpy(genre, "Genre film tidak di ketahui");
                }
                if (user.genrefilm == 1)
                {
                    strcpy(genre, "Action");
                }
                if (user.genrefilm == 2)
                {
                    strcpy(genre, "Horror");
                }
                if (user.genrefilm == 3)
                {
                    strcpy(genre, "Romance");
                }
                if (user.genrefilm == 4)
                {
                    strcpy(genre, "Thriller");
                }
                if (user.genrefilm == 5)
                {
                    strcpy(genre, "Drama");
                }
                printf("Nama Pembeli        : %s\n", user.nama);
                printf("Genre Film          : %s\n", genre);
                printf("Judul Film          : %s\n", user.judulfilm);
                printf("Harga               : %d\n", user.hargatiket);
                printf("Tahun Produksi Film : %d\n", user.tahunfilm);
                if (user.bayar == 1)
                {
                    if (user.ATM == 1)
                    {
                        strcpy(metode, "ATM (BCA) (NO REK : 09876543234567890)");
                    }
                    if (user.ATM == 2)
                    {
                        strcpy(metode, "ATM (Mandiri) (NO REK : 12345678987654321)");
                    }
                    if (user.ATM == 3)
                    {
                        strcpy(metode, "ATM (BRI) (NO REK : 00998877665544332)");
                    }
                    if (user.ATM == 4)
                    {
                        strcpy(metode, "ATM (BNI) (NO REK : 11223344556677889)");
                    }
                }
                else
                {
                    strcpy(metode, "Transaksi Gagal");
                }
                printf("Metode Pembayaran   : %s\n\n", metode);
                i++;
            }
        }
        fclose(f_datauser);
        system("pause");
        menuuser(); // rekursif
        break;
    }
    case 4:
    {
        system("cls");
        main();
        break;
    }
    default:
    {
        printf("Pilihan Anda Salah\nSilahkan Coba Lagi\n\n");
        system("pause");
        system("cls");
        menuuser(); // rekursif
        break;
    }
    }
    }
}

void userlogin()
{
    int berhasil = 0;
    system("cls");
    printf("====LOGIN USER====\n\n");
    FILE *f_datauser;
    f_datauser = fopen("User.dat", "rb");
    printf(" Username: ");
    gets(uname);
    printf(" Password: ");
    gets(pass);
    system("cls");
    while (fread(&user, sizeof(user), 1, f_datauser) == 1)
    {
        if (strcmp(uname, user.nama) == 0 && strcmp(pass, user.password) == 0) // searching binary
        {
            fclose(f_datauser);
            berhasil = 1;
            menuuser(); // pergi ke fungsi void menu user
        }
    }
    if (berhasil == 0)
    {
        printf("\n\n Login Gagal, Silahkan Coba Lagi\n\n");
        system("pause");
        userlogin(); // rekursif
    }
}

int main()
{
    printf("=====SELAMAT DATANG DI D'TICS=====\n");
    printf("=================MENU==================\n");
    printf("1. Login Admin\n");
    printf("2. Login User\n");
    printf("3. Registrasi User\n");
    printf("4. Exit\n");
    printf("Masukan pilihan menu : ");
    scanf("%d", &menu);
    getchar();
    switch (menu)
    {
    case 1:
        menuadmin(); // pergi ke fungsi void menu admin
        break;
    case 2:
        userlogin(); // pergi ke fungsi void user login
        break;
    case 3:
        regis(); // pergi ke fungsi void regis
        system("cls");
        break;
    case 4:
        exit(0);
        break;
    default:
        printf("Pilihan anda salah, mohon coba kembali\n");
        main(); // rekursif
        break;
    }
}
