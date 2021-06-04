# Mesh Generation

## Assets

### Download

Inside the "Assets" folder, place 2d assets as images (e.g.: Assets/taz.png) and 3d assets as directories (e.g.: Assets/knee/).

Note: 3d assets in this project are lists of .pgm files. To generate pgm's from other 3d formats use [ImageJ](https://imagej.nih.gov/ij/).

- [Drive Assets link](https://drive.google.com/drive/folders/1N1XqlrUMsS88whSzm2M1pCvgt6qu6EG1?usp=sharing)

- [Old link](https://drive.google.com/folderview?id=0B2Pp8Zn2YoIoT21OMmg0Sl9oSVE&usp=sharing)

### Cases

To prevent entering values manually, the YAML file `cases/base_cases.yml` was created, containing predefined cases (assets + parameters).

Make changes here in order to preserve and version good parameters for each asset.

## Build

### Linux:
```
$ make
```

## Execute

### Linux:
- Install [Ansible](https://docs.ansible.com/ansible/latest/installation_guide/intro_installation.html).
- Check the task definitions inside the `ansible-playbook.yml` file.
- Example:
```
$ ansible-playbook ansible-playbook.yml --tags="default" --extra-vars "case=wingdragon"
```