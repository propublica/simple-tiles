Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/trusty64"
  config.vm.provision "shell", inline: %Q{
  sudo apt-get -y update
  sudo apt-get -y install libgdal-dev libcairo2-dev libpango1.0-dev libosmesa6-dev libglu1-mesa-dev valgrind
  }
end
