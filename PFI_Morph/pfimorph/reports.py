import math

from jinja2 import Environment, FileSystemLoader, select_autoescape

from pfimorph_wrapper import util 

def format_number(num):
    # Can't compute log10(0)
    if (int(num) == 0):
        return "0"

    # Count number of digits in the number
    num_digits = int(math.ceil(math.log10(abs(num))))

    # Round to 3 sig figs
    round_place = -(num_digits - 3)
    rounded = round(num, round_place)

    # Add in thousands separators and trim decimals
    return "{:,.0f}".format(rounded)

def write_report(fname, stat_pairs):
    """
    Generate a HTML report using Jinja2
    """
    # Set up a Jinja templating environment, including a custom
    # format function for numbers
    env = Environment(
        loader=FileSystemLoader("templates"),
        autoescape=select_autoescape(['html', 'xml']))
    env.filters['format_number'] = format_number

    template = env.get_template('report.html')
    html = template.render(
        source_name=stat_pairs[0].source_name,
        stat_pairs=stat_pairs,
        columns=stat_pairs[0].COLUMNS)

    with open(fname, 'w') as f:
        f.write(html) 

def make_stat_table(N, indices, stat_pairs):
    """
    Make an NxN table of morph results. For the lower triangular portion,
    just swap source and target, since all morphs are done bidirectionally
    """
    table = [[None] * N for i in xrange(N)]
    for (i, j), stat_pair in zip(indices, stat_pairs):
        if i == j:
            table[i][j] = stat_pair
        else:
            table[i][j] = stat_pair
            table[j][i] = stat_pair.swapped
    return table

def write_many_reports(models, table):
    """
    Make the reports by iterating over the rows in the table from
    make_stat_table()
    """
    N = len(models)
    for i in xrange(N):
        source = util.get_short_name(models[i])
        report_fname = "Reports/{}_all.html".format(source)
        write_report(report_fname, table[i])
