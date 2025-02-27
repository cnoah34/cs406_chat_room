<template>
    <div class="container">
        <h1>Sign Up</h1>
        <form @submit.prevent="createUser" class="fields">
            <input type="text" id="username" placeholder="Username" v-model="formData.username">
            <input type="password" id="password" placeholder="Password" v-model="formData.password"
                        @input="checkRequirements">
            <button type="submit">Sign up</button>
        </form>
        <div v-if="passwordRequirements.length" class="password-requirements">
            <p style="padding-left: 15px">Password requirements:</p>
            <ul>
                <li
                    v-for="(requirement, index) in passwordRequirements"
                    :key="index"
                    :style="{ color: requirement.isValid ? 'green' : 'red' }"
                >
                    {{ requirement.text }}
                </li>
            </ul>
        </div>
        <p>Already have an account? <a style="color: var(--vue-green)">Sign in</a></p>
        <div v-if="errorMessage" class="error">{{ errorMessage }}</div>
        <div v-if="successMessage" class="success">{{ successMessage }}</div>
    </div>
</template>

<script>
    import axios from 'axios'

    export default {
        data() {
            return {
                formData: {
                    username: "",
                    password: ""
                },
                passwordRequirements: [
                    { text: "At least 8 characters", isValid: false },
                    { text: "At least one uppercase character", isValid: false },
                    { text: "At least one lowercase character", isValid: false },
                    { text: "At least one number", isValid: false },
                    { text: "At least one special character (#?!@$%^&*-)", isValid: false },
                ],
                errorMessage: "",
                successMessage: ""
            };
        },
        methods: {
            // Function for live password checking, seperate from actual validation
            checkRequirements() {
                const password = this.formData.password

                this.passwordRequirements[0].isValid = password.length >= 8
                this.passwordRequirements[1].isValid = /[A-Z]/.test(password)
                this.passwordRequirements[2].isValid = /[a-z]/.test(password)
                this.passwordRequirements[3].isValid = /\d/.test(password)
                this.passwordRequirements[4].isValid = /[#?!@$%^&*-]/.test(password)
            },

            // Checks if the password meets all requirements
            validatePassword(password) {
                // Min 8 characters, one uppercase, one lowercase, one number, one special character
                const passwordRegex = /^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*-]).{8,}$/
                                     
                return passwordRegex.test(password)
            },
            
            async createUser() {
                this.errorMessage = ""
                this.successMessage = ""

                if (!this.validatePassword(this.formData.password)) {
                    this.errorMessage = "Password requirements not met"
                    return
                }

                try {
                    const response = await axios.post("http://localhost:8080/users", this.formData)

                    console.log("User created: ", response.data)
                    this.successMessage = "User successfully created"
                    this.errorMessage = ""
                }
                catch (error) {
                    console.error("Error creating user: ", error)

                    if (error.response) {
                        if (error.response.data.error && error.response.data.error.includes("already exists")) {
                            this.errorMessage = "Username already exists"
                        }
                        else {
                            this.errorMessage = "An unexpected error occurred"
                        }
                    }
                    else {
                        this.errorMessage = "Failed to connect to the server"
                    }
                }
            }
        }
    };
</script>

<style>
.container {
    width: 400px;
    min-height: 500px;
    position: relative;
    display: flex;
    flex-direction: column;
    justify-content: center;
    align-items: center;
    background-color: var(--foreground);
    border: 4px solid var(--vue-green);
    padding: 20px;
}

.password-requirements {
    margin-bottom: 50px;
}

.fields {
    display: flex;
    flex-direction: column;
    justify-content: center;
    align-items: center;
    margin-top: 20px;
    margin-bottom: 20px;
}

.fields input {
    width: 300px;
    height: 40px;
    font-size: 12pt;
    margin-bottom: 10px;
}

.fields button {
    width: 70px;
    height: 30px;
    font-size: 11pt;
    margin-top: 10px;
}

h1 {
    color: var(--vue-green);
    margin-top: 20px;
}

</style>
