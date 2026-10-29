echo ===== SORU 1: yonlendirme =====
echo Isletim Sistemleri > out.txt
cat < out.txt > kopya.txt
cat kopya.txt
cat < yok_dosya.txt
echo ===== SORU 2: pipe =====
echo elma armut kiraz | wc -w
seq 1 5 | sort -r | head -n 2
seq 1 100000 | tail -n 1
cat < out.txt | tr a-z A-Z
echo ===== SORU 3: kosullu calistirma =====
true && echo dogru
false && echo BU_SATIR_YAZILMAMALI
false || echo yedek_calisti
false && echo x || echo y
cat < yok_dosya.txt || echo hata_yakalandi
echo abc | grep -q xyz || echo pipe_kodu_dogru
echo ===== SORU 4: komut listesi =====
echo bir ; echo iki ; echo uc
false ; echo yine_de_calisir
rm out.txt kopya.txt ; echo ===== BITTI =====
