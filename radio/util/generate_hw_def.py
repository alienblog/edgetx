
from os import path
import sys
import re
import json
import argparse
import jinja2

MAX_POTS = 4
MAX_SLIDERS = 4
MAX_EXTS = 4

class Switch:

    TYPE_2POS = '2POS'
    TYPE_3POS = '3POS'
    TYPE_ADC  = 'ADC'
    
    def __init__(self, name, sw_type, flags):
        self.name = name
        self.type = sw_type
        self.flags = flags

class Switch2POS(Switch):
    def __init__(self, name, gpio, pin, flags=0):
        super(Switch2POS, self).__init__(name, Switch.TYPE_2POS, flags)
        self.gpio = gpio
        self.pin = pin
        
class Switch3POS(Switch):
    def __init__(self, name, gpio_high, pin_high, gpio_low, pin_low, flags=0):
        super(Switch3POS, self).__init__(name, Switch.TYPE_3POS, flags)
        self.gpio_high = gpio_high
        self.pin_high = pin_high
        self.gpio_low = gpio_low
        self.pin_low = pin_low

class SwitchADC(Switch):
    def __init__(self, name, adc_input, flags=0):
        super(SwitchADC, self).__init__(name, Switch.TYPE_ADC, flags)
        self.adc_input = adc_input
        
class ADCInput:

    TYPE_STICK  = 'STICK'
    TYPE_POT    = 'POT'
    TYPE_SLIDER = 'SLIDER'
    TYPE_EXT    = 'EXT'
    TYPE_SWITCH = 'SWITCH'
    TYPE_SWITCH = 'SWITCH'
    TYPE_BATT   = 'BATT'

    def __init__(self, name, adc_input_type, gpio, pin, channel):
        self.name = name
        self.type = adc_input_type
        self.gpio = gpio
        self.pin = pin
        self.channel = channel

def prune_dict(d):
    # ret = {}
    # for k, v in d.items():
    #     if v is not None:
    #         ret[k] = v
    # return ret
    return d

class DictEncoder(json.JSONEncoder):

    def default(self, obj):
        if isinstance(obj, Switch):
            return prune_dict(obj.__dict__)
        if isinstance(obj, ADCInput):
            return prune_dict(obj.__dict__)
        if isinstance(obj, ADC):
            return prune_dict(obj.__dict__)

        # Let the base class default method raise the TypeError
        return json.JSONEncoder.default(self, obj)


def open_file(filename):
    
    if filename and not filename == '-':
        return open(filename)
    else:
        return sys.stdin
    
def parse_hw_defs(filename):

    hw_defs = {}

    with open_file(filename) as file:
        for line in file.readlines():
            m = re.match(r'#define ([^\s]*)\s*(.*)', line.rstrip())
            name = m.group(1)
            value = m.group(2)
            if value.isnumeric():
                value = int(value)
            elif not value:
                value = None
            hw_defs[name] = value

    return hw_defs

def AZ_seq():
    return [chr(i) for i in range(ord('A'), ord('Z') + 1)]

# switches from A to Z
def parse_switches(hw_defs, adc_parser):

    switches = []

    for s in AZ_seq():

        name = f'S{s}'

        reg = f'SWITCHES_GPIO_REG_{s}'
        reg_high = f'{reg}_H'
        reg_low = f'{reg}_L'

        pin = f'SWITCHES_GPIO_PIN_{s}'
        pin_high = f'{pin}_H'
        pin_low = f'{pin}_L'

        adc_input_name = f'S{s}'

        if reg in hw_defs:
            # 2POS switch
            reg = hw_defs[reg]
            pin = hw_defs[pin]
            switches.append(Switch2POS(name, reg, pin))
        elif (reg_high in hw_defs) and (reg_low in hw_defs):
            # 3POS switch
            reg_high = hw_defs[reg_high]
            pin_high = hw_defs[pin_high]
            reg_low = hw_defs[reg_low]
            pin_low = hw_defs[pin_low]
            switches.append(Switch3POS(name, reg_high, pin_high, reg_low, pin_low))
        else:
            # ADC switch
            if adc_parser.find_input(adc_input_name):
                switches.append(SwitchADC(name, adc_input_name))

    return switches

class ADC:
    def __init__(self, name, adc):
        self.name = name
        self.adc = adc
        self.adc_inputs = []

    def add_input(self, adc_input):
        if adc_input is not None:
            self.adc_inputs.append(adc_input)

    def find_input(self, name):
        for i in self.adc_inputs:
            if i.name == name:
                return i
        return None
    

class ADCInputParser:

    ADC_MAIN = 'MAIN'
    ADC_EXT = 'EXT'

    STICKS = ['RV','RH','LH','LV']

    ADC_INPUTS = {
        ADCInput.TYPE_STICK: {
            'range': ['RV','RH','LH','LV'],
            'suffix': 'STICK_{}',
            'name': '{}',
        },
        ADCInput.TYPE_POT: {
            'range': range(1, MAX_POTS + 1),
            'suffix': 'POT{}',
            'name': 'P{}',
        },
        ADCInput.TYPE_SLIDER: {
            'range': range(1, MAX_SLIDERS + 1),
            'suffix': 'SLIDER{}',
            'name': 'SL{}',
        },
        ADCInput.TYPE_EXT: {
            'range': range(1, MAX_EXTS + 1),
            'suffix': 'EXT{}',
            'name': 'EXT{}',
        },
        ADCInput.TYPE_SWITCH: {
            'range': AZ_seq(),
            'suffix': 'SW{}',
            'name': 'S{}',
        },
    }
    

    def __init__(self, hw_defs):
        self.hw_defs = hw_defs
        self.regs = self._parse_regs()
        self.adcs = []

    def _parse_regs(self):
        
        regs = {}
        for reg in AZ_seq():
            reg = f'GPIO{reg}'
            pins_def = f'ADC_{reg}_PINS'
            if pins_def in self.hw_defs:
                regs[reg] = self.hw_defs[pins_def]

        return regs

    def _find_adc(self, channel_def):

        if (self.ext_list is None) or (channel_def not in self.ext_list):
            return 0 # self.ADC_MAIN
        else:
            return 1 # self.ADC_EXT
        
    def _find_gpio(self, pin):
        gpio = None
        for reg, pins_def in self.regs.items():
            if pins_def and (pin in pins_def):
                gpio = reg

        return gpio

    def _parse_adcs(self):

        adcs = []

        adc_main = self.hw_defs.get('ADC_MAIN')
        if adc_main is None:
            # SPI ADC not yet supported (X12S only?)
            return []

        adcs.append(ADC('MAIN', adc_main))
    
        adc_ext = self.hw_defs.get('ADC_EXT')
        self.ext_list = self.hw_defs.get('ADC_EXT_CHANNELS')

        if adc_ext:
            adcs.append(ADC('EXT', adc_ext))

        return adcs

    def _parse_vbat(self):

        channel_def = 'ADC_CHANNEL_BATT'
        vbat = self.hw_defs.get(channel_def)
        if vbat:
            adc = self._find_adc(channel_def)
            return (adc, ADCInput('VBAT', 'BATT', None, None, vbat))

        return (None, None)

    def _parse_input_type(self, input_type, name, suffix):

        pin_def = f'ADC_GPIO_PIN_{suffix}'
        pin = self.hw_defs[pin_def]
        gpio = self._find_gpio(pin_def)

        # check if 'pin' is maybe an alias
        alias = self.hw_defs.get(pin)
        if alias is not None:
            alias_gpio = self._find_gpio(pin)
            gpio = alias_gpio if alias_gpio else gpio
            pin = alias
        
        channel_def = f'ADC_CHANNEL_{suffix}'
        channel = self.hw_defs[channel_def]
        adc = self._find_adc(channel_def)

        return (adc, ADCInput(name, input_type, gpio, pin, channel))

    def _add_input(self, adc, adc_input):
        if adc_input is not None:
            self.adcs[adc].add_input(adc_input)

    def parse_inputs(self):

        self.adcs = self._parse_adcs()

        for input_type, adc_input in self.ADC_INPUTS.items():
            try:
                for i in adc_input['range']:
                    name = adc_input['name'].format(i)
                    suffix = adc_input['suffix'].format(i)
                    self._add_input(*self._parse_input_type(input_type, name, suffix))
            except KeyError:
                pass

        self._add_input(*self._parse_vbat())
        return self.adcs

    def find_input(self, name):
        for adc in self.adcs:
            i = adc.find_input(name)
            if i is not None:
                return i
        return None


def parse_defines(filename):

    hw_defs = parse_hw_defs(filename)
    out_defs = {}

    # parse ADC first, we might have switches using ADC
    adc_parser = ADCInputParser(hw_defs)
    adc_inputs = adc_parser.parse_inputs()
    out_defs["adcs"] = adc_inputs

    switches = parse_switches(hw_defs, adc_parser)
    out_defs["switches"] = switches

    print(json.dumps(out_defs, cls=DictEncoder))


def build_adc_index(adcs):

    i = 0
    index = {}
    for adc in adcs:
        for adc_input in adc.get('adc_inputs'):
            name = adc_input['name']
            index[name] = i
            i = i + 1

    return index

def generate_from_template(json_filename, template_filename):

    with open(json_filename) as json_file:
        with open(template_filename) as template_file:

            root_obj = json.load(json_file)
            adc_index = build_adc_index(root_obj.get('adcs'))

            env = jinja2.Environment(
                lstrip_blocks=True,
                trim_blocks=True
            )

            template_str = template_file.read()
            template = env.from_string(template_str)

            print(template.render(root_obj, adc_index=adc_index))

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Process hardware definitions')
    parser.add_argument('filename', metavar='filename', nargs='+')
    parser.add_argument('-i', metavar='input', choices=['json','defines'], default='json')
    parser.add_argument('-t', metavar='template')

    args = parser.parse_args()

    if args.i == 'defines':
        for filename in args.filename:
            parse_defines(filename)

    elif args.i == 'json':
        for filename in args.filename:
            generate_from_template(filename, args.t)
