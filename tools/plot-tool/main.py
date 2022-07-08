import os

import matplotlib.pyplot as plt
import matplotlib
import numpy as np
from matplotlib.ticker import FuncFormatter
import matplotlib.ticker as mticker


def draw_boxplot(files, storages, workloads):
    """

    :param files: Input files: storage1-w1, storage2-w1, ....
    :param storages: Comparing storage list
    :param workloads: Workload count (e.g. 4)
    """

    def format_num(x, _):
        if x == 0:
            return '0'
        return '$%.1f$$\\times$$10^{6}$' % (x / 1000000)

    def set_box_color(bp, color, linestyle):
        plt.setp(bp['boxes'], color=color, linestyle=linestyle)
        plt.setp(bp['whiskers'], color=color, linestyle=linestyle)
        plt.setp(bp['caps'], color=color, linestyle=linestyle)
        plt.setp(bp['medians'], color=color, linestyle=linestyle)

    matplotlib.rc("font", family='FangSong')
    data = []  # data[i][j]: data for storage i and workload j
    line_styles = ['solid', 'dotted', 'dashed', 'dashdot']
    for i in range(0, len(storages)):  # Storage
        data.append([])
        for j in range(0, len(workloads)):  # Workload
            data[i].append([])
            file = files[j * len(storages) + i]
            csv_data = pd.read_csv(file)  # TODO：Properties chosen: tps/delay
            # delay = filter(lambda x: x > 1100000.0, csv_data['tps'].map(lambda x: int(x)))
            delay = filter(lambda x: x < 20.0, csv_data['delay'].map(lambda x: x * 1000.0))
            data[i][j] = list(delay)
    plt.figure()
    plt.rcParams.update({'font.sans-serif': "Arial",
                         'mathtext.fontset': 'custom',
                         'mathtext.rm': 'Arial',
                         })
    meanpoint_props = dict(marker='.', markeredgecolor='black',
                           markerfacecolor='black')

    for i, storage_data in enumerate(data):
        pos = i - (len(storages) - 1) * 1.0 / 2
        bp = plt.boxplot(storage_data,
                         positions=np.array(range(len(workloads))) * len(storages) + pos * 0.6,
                         sym='', widths=0.5, showmeans=True,
                         meanprops=meanpoint_props, whis=999)
        set_box_color(bp, '#000000', line_styles[i])
    for i in range(0, len(storages)):
        plt.plot([], color='#000000', label=storages[i], linestyle=line_styles[i])
    plt.legend()

    plt.xticks(range(0, len(storages) * len(workloads), len(storages)), workloads)
    # plt.yscale('log', basey=2)
    plt.xlim(-2, len(storages) * len(workloads))
    # plt.ylim(0, 80)
    # plt.tight_layout()

    plt.xlabel('Workload类型')
    plt.ylabel('延时（单位：微秒）')
    plt.ticklabel_format(axis='y', style='scientific', scilimits=(0, 0))
    plt.gcf().subplots_adjust(bottom=0.1, left=0.1)
    # formatter = FuncFormatter(format_num)
    # plt.gca().yaxis.set_major_formatter(formatter)
    plt.ticklabel_format(axis='y', style='plain')
    plt.show()


def draw_curve(file):

    def format_num(x, pos):
        if x == 0:
            return '0'
        return '$%.1f$$\\times$$10^{6}$' % (x / 1e6)  # TODO：Scientific representation exponent change 5/6

    matplotlib.rc("font", family='FangSong')
    plt.rcParams.update({'font.sans-serif': "Fangsong",
                         'mathtext.fontset': 'custom',
                         'mathtext.rm': 'Fangsong',
                         'font.size': '9.5'
                         })
    plt.rcParams['figure.figsize'] = (8.0, 4.5)
    plt.rcParams['figure.dpi'] = 100
    plt.plot(dpi=100)
    # csv_data = pd.read_csv(file)
    # csv_data['time'] = csv_data['time'].map(lambda x: x / 1000.0)
    # time_throughput = filter(lambda t: True, list(zip(list(csv_data['time']), list(csv_data['tps']))))
    # time, throughput = zip(*time_throughput)
    #
    # model = make_interp_spline(time, throughput)
    # time_new = np.linspace(min(time), max(time), 100)
    # throughput_new = model(time_new)
    # print(np.percentile(throughput_new, 0.9))

    time_new = [0, 1, 5, 10, 15, 20]
    throughput_new = [2501158, 2270392, 1930292, 1652029, 1342839, 1107142]

    plt.plot(time_new, throughput_new, color='black', marker='.')

    for a, b in zip(time_new, throughput_new):
        if a == 0:
            plt.text(a + 0.7, b + 15000, b, ha='center', va='bottom', fontsize=9.5)
        else:
            plt.text(a + 0.4, b + 35000, b, ha='center', va='bottom', fontsize=9.5)

    # plt.xlabel('时间（秒）')
    plt.xlabel('数量')
    plt.ylabel('频率')
    plt.ylim(0, 3e6)
    # plt.xlim(0, 85)
    plt.xlim(0, 21)
    plt.grid()

    # plt.ylim(0, 2e6)
    formatter = FuncFormatter(format_num)
    plt.gca().yaxis.set_major_formatter(formatter)
    tick_spacing = 5  # x軸密集度
    plt.gca().xaxis.set_major_locator(mticker.MultipleLocator(tick_spacing))
    plt.legend()
    plt.tight_layout()
    plt.show()


def draw_histogram():
    def format_num(x, pos):
        if x == 0:
            return '0'
        return '$%.0f$$\\times$$10^{6}$' % (x / 1000000)

    def auto_label(rects):
        """
        Attach a text label above each bar displaying its height
        """
        for rect in rects:
            height = rect.get_height()
            ax.text(rect.get_x() + rect.get_width() / 2., 0.048 + height,
                    '%.0f' % height, ha='center', va='bottom')

    matplotlib.rc("font", family='FangSong')
    plt.rcParams.update({'font.sans-serif': "Fangsong",
                         'mathtext.fontset': 'custom',
                         'mathtext.rm': 'Fangsong',
                         'font.size': '9.5'
                         })
    plt.rcParams['figure.figsize'] = (8.0, 4.5)

    N = 4
    ind = np.arange(N)  # the x locations for the groups
    width = 0.285  # the width of the bars
    # plt.figure(dpi=200)
    fig, ax = plt.subplots(dpi=125)

    means1 = (1501158, 2955910, 5070775, 5803568) 
    means2 = (1136454, 2113569, 4330535, 4487514) 
    means3 = (1454047, 2410830, 4546473, 4838916) 
    # means4 = (0.860188, 1.265174, ) 

    # rects1 = ax.bar(ind - 1.5 * width, means1, width, )
    # rects2 = ax.bar(ind - 0.5 * width, means2, width,)
    # rects3 = ax.bar(ind + 0.5 * width, means3, width, )
    # rects4 = ax.bar(ind + 1.5 * width, means4, width)

    rects1 = ax.bar(ind - 1 * width, means1, width, )
    rects2 = ax.bar(ind - 0 * width, means2, width,)
    rects3 = ax.bar(ind + 1 * width, means3, width, )
    # rects4 = ax.bar(ind + 1.5 * width, means4, width)

    # add some text for labels, title and axes ticks
    ax.set_xlabel('类型')
    ax.set_ylabel('平均吞吐量（单位：操作/秒）')
    # ax.set_title('Scores by group and gender')
    ax.set_xticks(ind)
    ax.set_xticklabels(('A类型', 'B类型', 'C类型', 'D类型'))
    ax.legend((rects1[0], rects2[0], rects3[0]), ('A', 'B', 'C'))
    # plt.yscale('log', basey=2)
    auto_label(rects1)
    auto_label(rects2)
    auto_label(rects3)
    formatter = FuncFormatter(format_num)
    plt.gca().yaxis.set_major_formatter(formatter)
    plt.tight_layout()
    plt.show()


def draw_line():
    def format_num(x, pos):
        if x == 0:
            return '0'
        return '$%.1f$$\\times$$10^{6}$' % (x / 1e6)  # TODO：Scientific representation exponent change 5/6

    matplotlib.rc("font", family='FangSong')
    plt.rcParams.update({'font.sans-serif': "FangSong",
                         'mathtext.fontset': 'custom',
                         'mathtext.rm': 'FangSong',
                         })
    fig, ax = plt.subplots(dpi=125)
    x_data = [1, 5, 10, 20]
    ax.set_xticks(x_data)
    ax.set_xticklabels(('1', '5', '10', '20'))

    y_data_1 = (36.36, 131.13, 281.03, 549.09)  # a
    y_data_2 = (42.99, 128.83, 258.11, 497.86)  # b
    y_data_3 = (50.00, 147.17, 289.88, 543.81)  # c
    y_data_4 = (50.84, 168.34, 290.32, 541.86)  # d

    # print(np.percentile(throughput_new, 0.9))
    # plt.plot(time_new, throughput_new)
    plt.xlabel('大小')
    plt.ylabel('延时')
    plt.plot(x_data, y_data_1, 'o-', label='A类型')
    plt.plot(x_data, y_data_2, 'v-.', label='B类型')
    plt.plot(x_data, y_data_3, '^--', label='c类型')
    plt.plot(x_data, y_data_4, 's:', label='d类型')
    # plt.ylim(0, 2e6)
    # plt.xlim(0, 85)
    # plt.grid()

    # plt.ylim(0, 2e6)
    plt.legend()
    plt.tight_layout()
    plt.show()


def draw_multi_line():
    def format_num(x, pos):
        if x == 0:
            return '0'
        return '$%.0f$$\\times$$10^{6}$' % (x / 100000)

    def auto_label(rects):
        """
        Attach a text label above each bar displaying its height
        """
        for rect in rects:
            height = rect.get_height()
            ax.text(rect.get_x() + rect.get_width() / 2., 0.048 + height,
                    '%.2f' % height, ha='center', va='bottom')

    matplotlib.rc("font", family='FangSong')
    plt.rcParams.update({'font.sans-serif': "FangSong",
                         'mathtext.fontset': 'custom',
                         'mathtext.rm': 'FangSong',
                         })

    N = 4
    ind = np.arange(N)  # the x locations for the groups
    # width = 0.225  # the width of the bars
    # plt.figure(dpi=200)
    fig, ax = plt.subplots(dpi=125)
    means_1 = (1.491158, 1.303854, 1.183135, 0.935594)  
    means_2 = (23.5, 23.5, 23.6, 23.7)
    # add some text for labels, title and axes ticks
    ax.set_xlabel('个数')
    ax.set_ylabel('操作')
    # ax.set_title('Scores by group and gender')
    ax.set_xticks(ind)
    ax.set_xticklabels(('0', '1', '5', '10'))
    ax.plot(ind, means_1, 'o-b', label='吞吐量')
    ax.legend(loc="upper left")
    ax.set_ylim(0, 2)
    ax.set_xlim(-0.5, 3.5)

    ax2 = ax.twinx()
    ax2.set_ylabel('占用')
    ax2.plot(ind, means_2, '^-r', label='占用空间')
    ax2.legend(loc="upper right")
    ax2.set_ylim(0, 60)

    plt.rcParams['font.sans-serif'] = ['SimHei']
    plt.rcParams['axes.unicode_minus'] = False
    # plt.xlabel('模式图边数')
    # plt.ylabel('离线模式图分解耗时（单位：兆字节）')
    # formatter = FuncFormatter(format_num)
    # plt.gca().yaxis.set_major_formatter(formatter)
    plt.tight_layout()
    plt.show()


def get_csv_info(files):
    if len(files) == 2:
        csv_data_1 = pd.read_csv(files[0])
        csv_data_2 = pd.read_csv(files[1])
        data = list(zip(
            list(csv_data_1['tps']),
            list(csv_data_1['delay']),
            list(csv_data_2['tps']),
            list(csv_data_2['delay'])
        ))
        data = map(lambda t: (t[1] * t[0] + t[3] * t[2]) * 1000.0 / (t[0] + t[2]), data)
        a = np.array(list(data))
        print(np.average(a))
        print(np.percentile(a, 25.0))
        print(np.percentile(a, 50.0))
        print(np.percentile(a, 75.0))
        print(np.percentile(a, 90.0))
        print(np.percentile(a, 99.0))
        print(np.percentile(a, 99.9))
        print(max(a))
    else:
        csv_data = pd.read_csv(files[0])
        data = map(lambda t : t * 1000.0, list(csv_data['delay']))
        a = np.array(list(data))
        print(np.average(a))
        print(np.percentile(a, 25.0))
        print(np.percentile(a, 50.0))
        print(np.percentile(a, 75.0))
        print(np.percentile(a, 90.0))
        print(np.percentile(a, 99.0))
        print(np.percentile(a, 99.9))
        print(max(a))


def rename_files(path):
    for root, dirs, files in os.walk(path):
        for file_name in files:
            if "_" in file_name:
                index1 = file_name.index("_")
                index2 = file_name.index(".")
                new_name = file_name[:index1] + file_name[index2:]
                os.rename(root + "/" + file_name, root + "/" + new_name)


def count_refs(file):
    set = {}
    with open(file, encoding="utf-8") as f:
        for line in f.readlines():
            if set.get(line) is None:
                set[line] = 1
            else:
                set[line] += 1
                print(line)
    # print(set)
    print(len(set))


if __name__ == '__main__':
    # count_refs("./refs.txt")
    # rename_files("./csv")
    # draw_histogram()
    # draw_line()
    draw_curve("./csv/sheet2/basemap/curve/SET.csv")
    # draw_multi_line()
    # draw_curve('1.csv')
    # draw_boxplot(['1.csv', '2.csv', '3.csv', '4.csv',
    #                 '5.csv', '6.csv', '7.csv', '8.csv',
    #                 '01.csv', '02.csv', '03.csv', '04.csv',
    #                 '05.csv', '06.csv', '07.csv', '08.csv'
    #                 ],
    #                ['A', 'B', 'C', 'D'],
    #                ['A类型', 'B类型', 'C类型', 'D类型'])
