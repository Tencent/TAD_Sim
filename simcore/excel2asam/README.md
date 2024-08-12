# excel2asam

The purpose of this project is to convert excel to xosc and xodr files,<br>
define scene by Excel to generate scene file in OpenSCENARIO (.xosc) and OpenDRIVE (.xodr) format.

## coverage

The coverage of OpenSCENARIO V1.0.0, and most of V1.1.0; OpenDRIVE 1.4 and 1.5.

# Getting Started

## prerequisites

Git & Docker: should be installed.

# Usage
## build map_sdk_py
Recompilation is only required when the code changes.

```bash
## run excel2asam local

```bash
# Write the scene definition in Excel according to the format requirements.
cd /work/excel2asam
# for local running excel mode
python3 producer.py --input_mode=excel --input_data=../tests/testdata/virturalmapdemo.xlsm --pathdir_output=./output
```

## run excel2asam binary

```bash
# for linux binary
./excel2asam --input_mode=excel --input_data=/work/tests/testdata/virturalmapdemo.xlsm --pathdir_output=./output --pathdir_catalog=/work/excel2asam/catalogs
```

```powershell
# for windows binary
.\excel2asam.exe --input_mode=excel --input_data=D:\UGit\excel2asam\tests\testdata\virturalmapdemo.xlsm --pathdir_output=.\output --pathdir_catalog=.\Catalogs
```

# Running the tests

```bash
# execution testing
cd /work/tests
pytest test_xxx.py
```
