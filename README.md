# C++ kanji lists

Helpful command for re-ordering columns (ensures empty columns are preserved):
awk -F'[\t]' -v OFS="\t" '{print $1,$2,$4,$5,$3,$6,$7,$8,$9}'

Convert double-byte numbers to single byte (and remove 画):
cat file|tr '１２３４５６７８９０' '1234567890'|tr -d '画'
