<template>
    <div class='auth'>
        <h1 style='color: var(--vue-green);'>{{ isLogin ? 'Login' : 'Sign Up' }}</h1>
        <div class='form-container'>
            <form @submit.prevent='handleSubmit'>
                <div class='form-data'>
                    <label for='username'>Username</label>
                    <input type='text' id='username' v-model='formData.username'
                           placeholder='Enter your username' />

                    <label for='password'>Password</label>
                    <input type='password' id='password' v-model='formData.password'
                           placeholder='Enter your password' @input='checkRequirements' @focus='showPasswordRequirements = !isLogin'
                        @blur='hideChecker'/>

                    <div v-if='showPasswordRequirements && !isLogin' class='password-requirements'>
                        <p>Password must contain:</p>
                        <ul>
                            <li v-for='(item, index) in passwordRequirements' :key='index'
                                :class="{'error': !item.isValid, 'success': item.isValid}">
                                {{ item.text }}
                            </li>
                        </ul>
                    </div>
                </div>


                <button type='submit'>{{ isLogin ? 'Login' : 'Sign Up' }}</button>
            </form>


            <p v-if='result.message' :class="{'error': result.isError, 'success':
                                             !result.isError}">{{ result.message }}</p>
        </div>

        <p style='margin-top: auto'>
        <span @click='toggleForm' class='link'>{{ isLogin? 'Don\'t have an account? Sign Up' : 'Already have an account? Login' }}</span>
        </p>
    </div>
</template>

<script setup>
    import { ref } from 'vue'
    import axios from 'axios'
    import { useRouter } from 'vue-router'

    const router = useRouter()

    const isLogin = ref(true)
    const showPasswordRequirements = ref(false)

    const result = ref({
        message: '',
        isError: false
    })

    const formData = ref({
        username: '',
        password: ''
    })

    const passwordRequirements = ref([
        { text: 'At least 8 characters', isValid: false },
        { text: 'At least one uppercase character', isValid: false },
        { text: 'At least one lowercase character', isValid: false },
        { text: 'At least one number', isValid: false },
        { text: 'At least one special character (#?!@$%^&*)', isValid: false }
    ])

// Function to validate password while typing
    const checkRequirements = () => {
        const password = formData.value.password;

        passwordRequirements.value[0].isValid = password.length >= 8;
        passwordRequirements.value[1].isValid = /[A-Z]/.test(password);
        passwordRequirements.value[2].isValid = /[a-z]/.test(password);
        passwordRequirements.value[3].isValid = /\d/.test(password);
        passwordRequirements.value[4].isValid = /[#?!@$%^&*]/.test(password);

        showPasswordRequirements.value = true;
    };

    const hideChecker = () => {
        if (!formData.value.password) {
            showPasswordRequirements.value = false
        }
    }

    const toggleForm = () => {
        isLogin.value = !isLogin.value
        formData.value.password = ''
        result.value = { message: '', isError: false }
        showPasswordRequirements.value = false
    }
    
    const validatePassword = (password) => {
        const passwordRegex = /^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*]).{8,}$/
        return passwordRegex.test(password)
    }

    const handleSubmit = async () => {
        result.value = { message: '', isError: false }

        if (!isLogin.value && !validatePassword(formData.value.password)) {
            result.value = { message: 'Password does not meet requirements', isError: true }
            return
        }

        try {
            const response = isLogin.value
                ? await axios.post('http://localhost:8080/login', formData.value)
                : await axios.post('http://localhost:8080/users', formData.value)

            if (response.status == 200 || response.status == 204) {
                result.value = {
                    message: isLogin.value
                        ? 'Login successful'
                        : 'Sign up successful, please login',
                    isError: false
                }
                    
                if (isLogin.value) {
                    const token = response.data.token

                    if (token) {
                        localStorage.setItem('token', token)
                        router.push('/home')
                    }
                    else {
                        result.value = { message: 'Token missing in response', isError: true }
                    }
                }
            }
        }
        catch (error) {
            if (error.response) {
                result.value = {
                    message: error.response.data.error || 'An unexpected error occurred',
                    isError: true
                }
            }
            else {
                result.value = { message: 'Failed to connect to the server', isError: true }
            }
        }
    }
</script>


<style scoped>
.auth {
    display: flex;
    flex-direction: column;
    align-items: center;
    height: 50vh;
    width: 25vw;
    padding: 20px;
    border: solid;
    border-width: 3px;
    border-color: var(--vue-green);
    background-color: var(--foreground);
}

.form-container {
    width: 80%;
}

.form-data {
    display: flex;
    flex-direction: column;
}

.form-data label {
    font-size: 14pt;
    text-align: left;
    margin-top: 10px;
}

.form-data input {
    height: 40px;
    font-size: 14pt;
    justify-content: center;
    align-items: center;
}

button {
    width: 100%;
    font-size: 14pt;
    padding: 10px;
    background-color: var(--vue-green);
    border: none;
    cursor: pointer;
    margin-top: 20px;
    margin-bottom: 20px;
}

button:hover {
    background-color: var(--vue-green);
}

.link {
    font-size: 12pt;
    color: var(--vue-green);
    cursor: pointer;
    text-decoration: underline;
}

.link:hover {
    text-decoration: none;
}

.error {
    color: red;
    font: bold;
}

.success {
    color: var(--vue-green);
    font: bold;
}


.password-requirements {
    position: absolute;
    top: 50%; /* Adjust as needed */
    left: 65%; /* Positions the box to the right of the password input */
    padding: 10px;
    border: 1px solid #ddd;
    background-color: var(--foreground);
    width: 300px; /* Adjust width as needed */
    box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
    z-index: 10;
}

.password-requirements p {
    font-size: 14pt;
    font-weight: bold;
    margin-bottom: 5px;
}

.password-requirements li {
    text-align: left;
    font-size: 12pt;
}

</style>
