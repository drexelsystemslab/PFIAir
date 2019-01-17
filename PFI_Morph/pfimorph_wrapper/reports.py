import math
from jinja2 import Environment, FileSystemLoader, select_autoescape

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
