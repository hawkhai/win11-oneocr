"""Canonical requested options; zero aliases retain the portable API contract."""
FIELDS=('max_lines','resize_width','resize_height')


def requested_options(value=None):
    value={} if value is None else value
    if not isinstance(value,dict) or set(value)-set(FIELDS):raise ValueError('Unknown recognition options')
    result={k:value.get(k,0) for k in FIELDS}
    if any(type(v) is not int for v in result.values()):raise ValueError('Recognition options must be integers')
    if not 0<=result['max_lines']<=1000:raise ValueError('Invalid max_lines')
    w,h=result['resize_width'],result['resize_height']
    if (w,h)!=(0,0) and not (32<=w<=8192 and 32<=h<=8192):raise ValueError('Invalid resize dimensions')
    return result


def effective_options(value=None):
    result=requested_options(value)
    if result['max_lines']==0:result['max_lines']=1000
    if result['resize_width']==result['resize_height']==0:result.update(resize_width=1152,resize_height=768)
    return result


def option_signature(value=None):return tuple(requested_options(value)[k] for k in FIELDS)


def cli_options(value=None):
    v=requested_options(value);args=[]
    if v['max_lines']:args+=['--max-lines',str(v['max_lines'])]
    if v['resize_width']:args+=['--resize',str(v['resize_width'])+'x'+str(v['resize_height'])]
    return args
