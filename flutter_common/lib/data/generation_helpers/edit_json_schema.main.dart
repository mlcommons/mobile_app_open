import 'dart:convert';
import 'dart:io';

const fileNameEnv = 'schemaPath';

void makeNullable(Map<String, dynamic> fieldDescription) {
  // print(jsonEncode(fieldDescription));
  dynamic originalType;
  var originalKey = '';
  const possibleDescriptions = <String>['type', '\$ref'];
  for (var item in possibleDescriptions) {
    if (!fieldDescription.containsKey(item)) {
      continue;
    }
    if (originalKey != '') {
      throw 'several types defined in ${jsonEncode(fieldDescription)}';
    }
    originalType = fieldDescription.remove(item);
    originalKey = item;
  }
  if (originalKey == '') {
    throw 'no type found in ${jsonEncode(fieldDescription)}';
  }
  fieldDescription['oneOf'] = [
    {originalKey: originalType},
    {'type': 'null'}
  ];
}

Future<void> main() async {
  if (!const bool.hasEnvironment(fileNameEnv)) {
    print('pass --define=$fileNameEnv=<value> to specify file to write');
    exit(1);
  }
  const filename = String.fromEnvironment(fileNameEnv);
  final fileContent = await File(filename).readAsString();

  final schema = jsonDecode(fileContent);
  final definitions = schema['definitions'];
  makeNullable(definitions['Meta']['properties']['upload_date']);
  makeNullable(definitions['Result']['properties']['performance_run']);
  makeNullable(definitions['Result']['properties']['accuracy_run']);
  makeNullable(definitions['Run']['properties']['throughput']);
  makeNullable(definitions['Run']['properties']['accuracy']);
  makeNullable(definitions['Run']['properties']['accuracy2']);
  makeNullable(definitions['Run']['properties']['loadgen_info']);
  (definitions['Run']['properties']['start_datetime'] as Map<String, dynamic>)
      .remove('format');

  final editedSchema = const JsonEncoder.withIndent('    ').convert(schema);
  await File(filename).writeAsString(editedSchema);
}
