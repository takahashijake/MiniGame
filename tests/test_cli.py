"""Run native input/persistence regressions in isolated working directories."""
import pathlib
import subprocess
import sys
import tempfile

executable = str(pathlib.Path(sys.argv[1]).resolve())


def run(commands, directory):
    return subprocess.run([executable], input=commands, text=True, cwd=directory,
                          capture_output=True, timeout=5, check=True).stdout


with tempfile.TemporaryDirectory() as directory:
    for commands in ('', 'Ada\n42\n', 'Ada\n42\nw\n', 'Ada\n42\nm\n'):
        run(commands, directory)  # EOF must terminate in exploration, battle and merchant.
    for seed in ('-1', '4294967296', '42junk', '1.5', '99999999999999999999999'):
        assert 'Invalid seed' in run(f'Ada\n{seed}\nq\n', directory), seed
    for seed in ('0', '4294967295'):
        output = run(f'Ada\n{seed}\nq\n', directory)
        assert 'Invalid seed' not in output and f'Seed: {seed}' in output, seed
    output = run('Ada\n42\nw\ns\na\nl\nq\n', directory)
    assert 'Run saved' in output and 'Run loaded' in output
    assert output.count('Rogue  [####################] 65/65 HP') == 3
    save = pathlib.Path(directory, '.minigame-save')
    assert save.read_text().startswith('MG2 42 "Ada"')
    output = run('Other\n0\nl\nq\n', directory)
    assert 'Run loaded' in output and 'Ada  ' in output and 'Seed: 42' in output
    save.write_text('invalid')
    assert 'invalid or incompatible' in run('Ada\n42\nl\nq\n', directory)
print('Native CLI regressions passed.')
