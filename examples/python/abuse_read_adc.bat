FOR /L %%A IN (1,1,100) DO (
  python simple.py -a -s -f bax1.csv
  SLEEP 2
)