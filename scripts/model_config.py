"""Read the small, documented subset of the local PipelineProto wire format.

Field numbers were verified against the fixed sample's embedded descriptors.
Unknown fields are retained by the container, and ignored by this reader.
"""
import struct

def fields(data):
    position=0;result={}
    def varint():
        nonlocal position
        value=0
        for shift in range(0,70,7):
            if position>=len(data):raise ValueError('Truncated protobuf varint')
            byte=data[position];position+=1;value|=(byte&127)<<shift
            if not byte&128:return value
        raise ValueError('Oversized protobuf varint')
    while position<len(data):
        tag=varint();number=tag>>3;wire=tag&7
        if not number:raise ValueError('Invalid protobuf field number')
        if wire==0:value=varint()
        else:
            size=varint() if wire==2 else {1:8,5:4}.get(wire)
            if size is None or size>len(data)-position:raise ValueError('Invalid protobuf field bounds')
            value=data[position:position+size];position+=size
        result.setdefault(number,[]).append((wire,value))
    return result

def single(message,number,wire,default=None):
    if number not in message:return default
    if len(message[number])!=1 or message[number][0][0]!=wire:raise ValueError('Unexpected protobuf field type or count')
    return message[number][0][1]

def settings(data,languages):
    pipeline=fields(data);characters={};rejection={}
    for wire,payload in pipeline.get(3,[]):
        if wire!=2:raise ValueError('Invalid character model configuration')
        model=fields(payload);name=single(model,1,2).decode('utf8')
        if name not in languages:raise ValueError('Unknown configured character model: '+name)
        record={'name':name}
        for number,key,default in [(7,'stride',4),(8,'pixels_per_space',12),(9,'start_shift',0),(10,'end_shift',0),(14,'top_k',30),(17,'keep_continuous_peak_blanks',0)]:
            value=single(model,number,0,default)
            record[key]=value-(1<<64) if value&(1<<63) else value
        for number,key,default in [(13,'minimum_probability',.0001),(15,'blank_threshold',.99),(16,'character_threshold',.05)]:
            value=single(model,number,5)
            record[key]=default if value is None else struct.unpack('<f',value)[0]
        if languages[name] in characters:raise ValueError('Duplicate character settings')
        characters[languages[name]]=record
    for wire,payload in pipeline.get(7,[]):
        if wire!=2:raise ValueError('Invalid rejection model configuration')
        model=fields(payload);name=single(model,1,2).decode('utf8')
        threshold=single(model,3,1);single_threshold=single(model,4,1)
        if threshold is None or single_threshold is None:raise ValueError('Missing rejection thresholds')
        rejection[name]={'threshold':struct.unpack('<d',threshold)[0],'single_threshold':struct.unpack('<d',single_threshold)[0]}
    if set(characters)!=set(languages.values()):raise ValueError('Incomplete configured language set')
    layout=fields(single(pipeline,13,2))
    layout_settings={'name':single(layout,1,2).decode('utf8'),'vertical_threshold':struct.unpack('<d',single(layout,3,1))[0],'horizontal_threshold':struct.unpack('<d',single(layout,4,1))[0]}
    classifier=fields(single(pipeline,20,2))
    classifier_settings={}
    for number,key in [(3,'script_margin'),(4,'handwriting_min'),(5,'handwriting_max'),(6,'flip_min'),(7,'flip_max'),(11,'nontext_margin')]:
        value=single(classifier,number,1)
        classifier_settings[key]=0. if value is None else struct.unpack('<d',value)[0]
    classifier_settings['handwriting_consecutive']=single(classifier,9,0)
    classifier_settings['script_consecutive']={}
    for wire,payload in classifier.get(10,[]):
        if wire!=2:raise ValueError('Invalid consecutive script configuration')
        entry=fields(payload)
        classifier_settings['script_consecutive'][single(entry,1,2).decode('utf8')]=single(entry,2,0)
    return {'character_settings':characters,'rejection_settings':rejection,'layout_settings':layout_settings,'classifier_settings':classifier_settings}
