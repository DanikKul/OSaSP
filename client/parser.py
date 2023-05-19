import os
import json


def parse_json(path: str) -> dict:
    with open(path) as conf:
        return json.load(conf)


def fix_json(config: dict):
    if config.get('address') is None:
        config['address'] = '127.0.0.1'
    if config.get('port') is None:
        config['port'] = 8080
