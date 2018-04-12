import BroControl.plugin
import BroControl.config

class Napatech(BroControl.plugin.Plugin):
    def __init__(self):
        super(Napatech, self).__init__(apiversion=1)

    def name(self):
        return 'napatech'

    def pluginVersion(self):
        return 1
    
    def init(self):

        # Use this plugin only if there is a Napatech interface in use
        for nn in self.nodes():
            if nn.type == 'worker' and nn.interface.startswith('napatech::'):
                return True
        
        return False
    
    def nodeKeys(self):

        return ['enable_software_deduplication', 'dedupe_lru_size', 'host_buffer_allowance']
    
    def options(self):

        return [('enable_software_deduplication', 'bool', True, 'Whether or not to use software packet deduplication.'),
                ('dedupe_lru_size', 'int', 1024, 'Size of deduplication lru.'),
                ('host_buffer_allowance', 'int', 100, 'Host buffer allowance.'))]

    def broctl_config(self):

        script = ''
        script += '# Settings for configuring Napatech interractions'
        script += '\nredef Napatech::enable_software_deduplication = {0};'.format(self.getOption('enable_software_deduplication'))
        script += '\nredef Napatech::dedupe_lru_size = {0};'.format(self.getOption('dedupe_lru_size'))
        script += '\nredef Napatech::host_buffer_allowance = {0};'.format(self.getOption('host_buffer_allowance'))
        
        return script
