king = 12
while king > 0:
    for queen in range(11):
        jack = 0
        while jack < 10:
            print king, queen, jack
            jack = jack + 1
    king = king - 1
