# FFT
Fast Fourier Transform  

Cooley&Tukey型@1965  
周波数間引き型(フーリエ変換後のフーリエ係数の並びが変わる)  

ポイント  
1) 関数値ベクトル`x(n)`のデータ点数`N`は2のべき乗とする  
2) 回転子`W_{k}_{n} = \exp{2*\pi*k*n}'
