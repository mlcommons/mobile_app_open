
import { ExtendedResult } from './extended-result.gen';

// key: key in the db index
// value: matching value from result data
const osList: Record<string, string> = {
    '01_android': 'android',
    '02_ios': 'ios',
    '03_windows': 'windows',
    // in case we need support for more operating systems
    '04': '',
    '05': '',
    '06': '',
}

export const osListReverse = generateReverseList(osList);

export function generateIndices(data: ExtendedResult): any {
    let result: any = {};
    result.osIs = {};
    for (const key in osList) {
        result.osIs[key] = data.environment_info.os_name == osList[key];
    }
    return result;
}

function generateReverseList(list: Record<string, string>): Record<string, string> {
    let result: Record<string, string> = {};
    for (const key in osList) {
        result[list[key]] = key;
    }
    return result;
}
