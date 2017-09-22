import matplotlib.pyplot as plt
import csv

plt.figure()

with open('8x8.csv', newline='') as csvfile:
    rdr = csv.reader(csvfile, delimiter=',', quotechar='|')
    header = next(rdr)
    values = []
    print(values)
    print(header)
    for row in rdr:
        plt.plot(row, label='label', linewidth=2)
        values.append(row)

    agents = [x[0] for x in values]
    obstacles = [x[1] for x in values]

    for i in range(2, len(header)):
        plt.figure()

        header_values = [x[i] for x in values]

        plt.plot(agents, header_values, label='label', linewidth=2)

        plt.xlabel('Num Agents')
        plt.ylabel(header[i])
        plt.title(header[i])
        plt.savefig(header[i] + '.pdf', format='pdf')
        plt.clf()
        plt.close()
        
    
        # for val in row.split(',')
        #     print(val)
        # print(', '.join(row))

# for i in range(num_strats):
#     plt.plot(track_round[i], label=strategy_names[i], linewidth=2)

# plt.ylim((0, 100))
# plt.xlim((0, 50))
plt.xlabel('Num rounds')
plt.ylabel('Population')
plt.title('Title')
# plt.legend(loc='best', prop={'size': 6})
plt.savefig('name.pdf', format='pdf')
plt.clf()
plt.close()