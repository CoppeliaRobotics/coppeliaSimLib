def flags(p):
    f = []
    if not p.flags.readable: f.append('notreadable')
    if not p.flags.writable: f.append('notwritable')
    if p.flags.removable: f.append('removable')
    if p.flags.constant: f.append('constant')
    if p.flags.silent: f.append('silent')
    if p.flags.modelhashexclude: f.append('modelhashexclude')
    if p.flags.deprecated: f.append('deprecated')
    constName = lambda n: f'sim_propertyinfo_{n}'
    return ' | '.join(map(constName, f)) if f else '0'

def propertyInfo(p):
    info = {}
    def outputInfo(keyName, attrName):
        if hasattr(p, attrName):
            if v := getattr(p, attrName):
                info[keyName] = v
    outputInfo('label', 'label')
    outputInfo('description', 'description')
    outputInfo('supersedes', 'supersedes')
    outputInfo('replacedBy', 'replaced_by')
    outputInfo('migrateTo', 'migrate_to')
    outputInfo('startSupport', 'start_support')
    outputInfo('endSupport', 'end_support')
    outputInfo('startDeprecated', 'start_deprecated')
    outputInfo('handleType', 'handle_type')
    outputInfo('enum', 'enum')
    return 'PropertyInfo({{' + '}, {'.join(f'"{k}", "{v}"' for k, v in info.items()) + '}})'
