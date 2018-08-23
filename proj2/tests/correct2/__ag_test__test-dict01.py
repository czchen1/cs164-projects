d1 = { 1 : "duck", 2: "goose" }
i = 1
y = d1[i]
print y
d1[3] = "cow"
print d1[3]

d2 = { "duck": 1, "goose": 2 }

print d2["duck"]
d2["horse"] = 3
print d2["duck"], d2["horse"]

d3 = { True: 1, False: 2 }

print d3[True]
d3[False] = 3
print d3[True], d3[False]
