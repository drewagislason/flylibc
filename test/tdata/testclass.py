#!/usr/bin/env python3

class Customer:
    """!
    @class Customer A class that defines attributes of a customer. Useful in many applications.

    Default attributes:
        name    full name of person
        age     age of person
    """

    def __init__(self, name, age):
        """!
        Initialize a customer. Requires name and age.

        Future versions will have date of birth (dob) and other fields.

        @param  name    full name
        @param  age     age in years (0-n)
        """
        self.name = name
        self.age = age

    def printme(self):
        """!
        Display name of Customer
        """
        print("  Hello my name is", self.name)

    def printage(self):
        """!
        print age of Customer
        """
        print("  I can't believe", self.name, "is already", self.age);

##!
# @class Airplane - A class for handling all things airplanes
#
# For now, it's designed for small aircraft. May support 737+ aircraft later
class Airplane:
    def __init__(self, make, model, year):
        self.make = make
        self.model = model
        self.year = year

    ##!
    #   display make/model of Airplane
    #   @return none
    def printme(self):
        print("  This airplane is a", self.make, self.model)

    ##!
    #   display year of Airplane
    def print_year(self):
        print("  Built in", self.year)

if __name__ == '__main__':
    print("Python class examples:")
    print("  Class Customer")
    customer = Customer("John", 36)
    customer.printme()
    customer.printage()

    print("  Class Airplane")
    airplane = Airplane("HondaJet", "HA-420", 2022)
    airplane.printme()
    airplane.print_year()
